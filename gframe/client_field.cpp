#include <stack>
#include "utils.h"
#include "game_config.h"
#include <IGUIWindow.h>
#include <IGUIStaticText.h>
#include <IGUIScrollBar.h>
#include <IGUIListBox.h>
#include <IGUIEditBox.h>
#include <IGUICheckBox.h>
#include <IGUIImage.h>
#include <IVideoDriver.h>
#include <ICameraSceneNode.h>
#include "game.h"
#include "client_field.h"
#include "client_card.h"
#include "duelclient.h"
#include "data_manager.h"
#include "image_manager.h"
#include "game.h"
#include "materials.h"
#include "core_utils.h"
#include "CGUIImageButton/CGUIImageButton.h"
#include "CGUITTFont/CGUITTFont.h"
#include "custom_skin_enum.h"
#include "fmt.h"

namespace ygo {

ClientField::ClientField() {
	panel = nullptr;
	hovered_card = nullptr;
	clicked_card = nullptr;
	highlighting_card = nullptr;
	hovered_controler = 0;
	hovered_location = 0;
	hovered_sequence = 0;
	selectable_field = 0;
	selected_field = 0;
	infimp_field = 0;
	infimp_check = false;
	deck_act[0] = deck_act[1] = false;
	grave_act[0] = grave_act[1] = false;
	remove_act[0] = remove_act[1] = false;
	extra_act[0] = extra_act[1] = false;
	pzone_act[0] = pzone_act[1] = false;
	conti_act = false;
	deck_reversed = false;
	conti_selecting = false;
	for(int p = 0; p < 2; ++p) {
		skills[p] = nullptr;
		mzone[p].resize(7, nullptr);
		szone[p].resize(8, nullptr);
	}
}

void ClientField::Clear() {
	auto ClearVector = [](auto& vec) {
		for(auto& pcard : vec)
			delete pcard;
		vec = {};
	};
	for(int i = 0; i < 2; ++i) {
		ClearVector(mzone[i]);
		ClearVector(szone[i]);
		mzone[i].resize(7, nullptr);
		szone[i].resize(8, nullptr);
		ClearVector(deck[i]);
		ClearVector(hand[i]);
		ClearVector(grave[i]);
		ClearVector(remove[i]);
		ClearVector(extra[i]);
	}
	ClearVector(limbo_temp);
	ClearVector(overlay_cards);
	if(skills[0]) {
		delete skills[0];
		skills[0] = nullptr;
	}
	if(skills[1]) {
		delete skills[1];
		skills[1] = nullptr;
	}
	overlay_cards.clear();
	extra_p_count[0] = 0;
	extra_p_count[1] = 0;
	player_desc_hints[0].clear();
	player_desc_hints[1].clear();
	chains.clear();
	activatable_cards.clear();
	queued_panel_confirm_cards.clear();
	summonable_cards.clear();
	spsummonable_cards.clear();
	msetable_cards.clear();
	ssetable_cards.clear();
	reposable_cards.clear();
	attackable_cards.clear();
	sort_list.clear();
	disabled_field = 0;
	panel = 0;
	hovered_card = 0;
	clicked_card = 0;
	highlighting_card = 0;
	hovered_controler = 0;
	hovered_location = 0;
	hovered_sequence = 0;
	infimp_field = 0;
	infimp_check = false;
	callbg_codes0.clear();
	callbg_codes1.clear();
	crossd_codes.clear();
	selectable_field = 0;
	selected_field = 0;
	deck_act[0] = deck_act[1] = false;
	grave_act[0] = grave_act[1] = false;
	remove_act[0] = remove_act[1] = false;
	extra_act[0] = extra_act[1] = false;
	pzone_act[0] = pzone_act[1] = false;
	conti_act = false;
	conti_selecting = false;
	deck_reversed = false;
}
void ClientField::Initial(uint8_t player, uint32_t deckc, uint32_t extrac) {
	ClientCard* pcard;
	for(uint32_t i = 0; i < deckc; ++i) {
		pcard = new ClientCard{};
		deck[player].push_back(pcard);
		pcard->owner = player;
		pcard->controler = player;
		pcard->location = LOCATION_DECK;
		pcard->sequence = i;
		pcard->position = POS_FACEDOWN_DEFENSE;
		pcard->UpdateDrawCoordinates(true);
	}
	for(uint32_t i = 0; i < extrac; ++i) {
		pcard = new ClientCard{};
		extra[player].push_back(pcard);
		pcard->owner = player;
		pcard->controler = player;
		pcard->location = LOCATION_EXTRA;
		pcard->sequence = i;
		pcard->position = POS_FACEDOWN_DEFENSE;
		pcard->UpdateDrawCoordinates(true);
	}
}
std::vector<ClientCard*>* ClientField::GetList(uint8_t location, uint8_t controler) {
	switch(location) {
	case LOCATION_DECK:
		return &deck[controler];
		break;
	case LOCATION_HAND:
		return &hand[controler];
		break;
	case LOCATION_MZONE:
		return &mzone[controler];
		break;
	case LOCATION_SZONE:
		return &szone[controler];
		break;
	case LOCATION_GRAVE:
		return &grave[controler];
		break;
	case LOCATION_REMOVED:
		return &remove[controler];
		break;
	case LOCATION_EXTRA:
		return &extra[controler];
		break;
	}
	return nullptr;
}
ClientCard* ClientField::GetCard(uint8_t controler, uint8_t location, size_t sequence, size_t sub_seq) {
	bool is_xyz = (location & LOCATION_OVERLAY) != 0;
	auto lst = GetList(location & (~LOCATION_OVERLAY), controler);
	if(!lst)
		return 0;
	if(is_xyz) {
		if(sequence >= lst->size())
			return 0;
		ClientCard* scard = (*lst)[sequence];
		if(scard && scard->overlayed.size() > sub_seq)
			return scard->overlayed[sub_seq];
		else
			return 0;
	} else {
		if(sequence >= lst->size())
			return 0;
		return (*lst)[sequence];
	}
}
void ClientField::AddCard(ClientCard* pcard, uint8_t controler, uint8_t location, uint32_t sequence) {
	pcard->controler = controler;
	pcard->location = location;
	pcard->sequence = sequence;
	float z_increase = gGameConfig->topdown_view ? 0.0f : 0.01f;
	switch(location) {
	case LOCATION_DECK: {
		if (sequence != 0 || deck[controler].empty()) {
			deck[controler].push_back(pcard);
			pcard->sequence = static_cast<uint32_t>(deck[controler].size() - 1);
		} else {
			deck[controler].push_back(0);
			for(auto i = deck[controler].size() - 1; i > 0; --i) {
				deck[controler][i] = deck[controler][i - 1];
				deck[controler][i]->sequence++;
				deck[controler][i]->curPos.Z += z_increase;
				deck[controler][i]->mTransform.setTranslation(deck[controler][i]->curPos);
			}
			deck[controler][0] = pcard;
			pcard->sequence = 0;
		}
		pcard->is_reversed = false;
		break;
	}
	case LOCATION_HAND: {
		hand[controler].push_back(pcard);
		pcard->sequence = static_cast<uint32_t>(hand[controler].size() - 1);
		break;
	}
	case LOCATION_MZONE: {
		mzone[controler][sequence] = pcard;
		break;
	}
	case LOCATION_SZONE: {
		szone[controler][sequence] = pcard;
		break;
	}
	case LOCATION_GRAVE: {
		grave[controler].push_back(pcard);
		pcard->sequence = static_cast<uint32_t>(grave[controler].size() - 1);
		break;
	}
	case LOCATION_REMOVED: {
		remove[controler].push_back(pcard);
		pcard->sequence = static_cast<uint32_t>(remove[controler].size() - 1);
		break;
	}
	case LOCATION_EXTRA: {
		if(extra_p_count[controler] == 0 || (pcard->position & POS_FACEUP)) {
			extra[controler].push_back(pcard);
			pcard->sequence = static_cast<uint32_t>(extra[controler].size() - 1);
		} else {
			extra[controler].push_back(0);
			auto p = extra[controler].size() - extra_p_count[controler] - 1;
			for(auto i = extra[controler].size() - 1; i > p; --i) {
				extra[controler][i] = extra[controler][i - 1];
				extra[controler][i]->sequence++;
				extra[controler][i]->curPos.Z += z_increase;
				extra[controler][i]->mTransform.setTranslation(extra[controler][i]->curPos);
			}
			extra[controler][p] = pcard;
			pcard->sequence = static_cast<uint32_t>(p);
		}
		if (pcard->position & POS_FACEUP)
			extra_p_count[controler]++;
		break;
	}
	}
}
ClientCard* ClientField::RemoveCard(uint8_t controler, uint8_t location, uint32_t sequence) {
	auto RemoveFromPile = [&](auto& pile) {
		float z_decrease = gGameConfig->topdown_view ? 0.0f : 0.01f;
		auto pcard = pile[controler][sequence];
		for(size_t i = sequence; i < pile[controler].size() - 1; ++i) {
			pile[controler][i] = pile[controler][i + 1];
			pile[controler][i]->sequence--;
			pile[controler][i]->curPos.Z -= z_decrease;
			pile[controler][i]->mTransform.setTranslation(pile[controler][i]->curPos);
		}
		pile[controler].pop_back();
		return pcard;
	};
	ClientCard* pcard = nullptr;
	switch (location) {
	case LOCATION_DECK: {
		pcard = RemoveFromPile(deck);
		break;
	}
	case LOCATION_HAND: {
		pcard = hand[controler][sequence];
		for (size_t i = sequence; i < hand[controler].size() - 1; ++i) {
			hand[controler][i] = hand[controler][i + 1];
			hand[controler][i]->sequence--;
		}
		hand[controler].pop_back();
		break;
	}
	case LOCATION_MZONE: {
		std::swap(pcard, mzone[controler][sequence]);
		break;
	}
	case LOCATION_SZONE: {
		std::swap(pcard, szone[controler][sequence]);
		break;
	}
	case LOCATION_GRAVE: {
		pcard = RemoveFromPile(grave);
		break;
	}
	case LOCATION_REMOVED: {
		pcard = RemoveFromPile(remove);
		break;
	}
	case LOCATION_EXTRA: {
		pcard = RemoveFromPile(extra);
		if (pcard->position & POS_FACEUP)
			extra_p_count[controler]--;
		break;
	}
	}
	pcard->location = 0;
	return pcard;
}
void ClientField::UpdateCard(uint8_t controler, uint8_t location, uint32_t sequence, const uint8_t* data, uint32_t len) {
	ClientCard* pcard = GetCard(controler, location, sequence);
	if(pcard) {
		if(mainGame->dInfo.compat_mode)
			len = BufferIO::Read<uint32_t>(data);
		pcard->UpdateInfo(CoreUtils::Query{ data, mainGame->dInfo.compat_mode, len, mainGame->dInfo.legacy_race_size });
	}
}
void ClientField::UpdateFieldCard(uint8_t controler, uint8_t location, const uint8_t* data, uint32_t len) {
	auto lst = GetList(location, controler);
	if(!lst)
		return;
	CoreUtils::QueryStream stream{ data, mainGame->dInfo.compat_mode, len, mainGame->dInfo.legacy_race_size };
	auto cit = lst->begin();
	for(const auto& query : stream.GetQueries()) {
		auto pcard = *cit++;
		if(pcard)
			pcard->UpdateInfo(query);
	}
}
void ClientField::ClearCommandFlag() {
	auto ClearFlag = [](const std::vector<ClientCard*>& map) { for(auto& pcard : map) pcard->cmdFlag = 0; };
	ClearFlag(activatable_cards);
	ClearFlag(summonable_cards);
	ClearFlag(spsummonable_cards);
	ClearFlag(msetable_cards);
	ClearFlag(ssetable_cards);
	ClearFlag(reposable_cards);
	ClearFlag(attackable_cards);
	conti_cards.clear();
	deck_act[0] = deck_act[1] = false;
	grave_act[0] = grave_act[1] = false;
	remove_act[0] = remove_act[1] = false;
	extra_act[0] = extra_act[1] = false;
	pzone_act[0] = pzone_act[1] = false;
	conti_act = false;
}
void ClientField::ClearSelect() {
	for(auto& pcard : selectable_cards) {
		pcard->is_selectable = false;
		pcard->is_selected = false;
	}
}
void ClientField::ClearChainSelect() {
	for(auto& pcard : activatable_cards) {
		pcard->cmdFlag = 0;
		pcard->chain_code = 0;
		pcard->is_selectable = false;
		pcard->is_selected = false;
	}
	conti_cards.clear();
	deck_act[0] = deck_act[1] = false;
	grave_act[0] = grave_act[1] = false;
	remove_act[0] = remove_act[1] = false;
	extra_act[0] = extra_act[1] = false;
	pzone_act[0] = pzone_act[1] = false;
	conti_act = false;
}
// needs to be synchronized with EGET_SCROLL_BAR_CHANGED
void ClientField::ShowSelectCard(bool buttonok, bool chain) {
	size_t startpos;
	size_t ct;
	if(selectable_cards.size() <= 5) {
		startpos = 30 + 125 * (5 - selectable_cards.size()) / 2;
		ct = selectable_cards.size();
	} else {
		startpos = 30;
		ct = 5;
	}
	for(size_t i = 0; i < ct; ++i) {
		auto& curstring = mainGame->stCardPos[i];
		auto& curcard = selectable_cards[i];
		curstring->enableOverrideColor(false);
		// image
		if (curcard->code)
			mainGame->imageLoading[mainGame->btnCardSelect[i]] = curcard->code;
		else if (conti_selecting)
			mainGame->imageLoading[mainGame->btnCardSelect[i]] = curcard->chain_code;
		else
			mainGame->btnCardSelect[i]->setImage(mainGame->imageManager.tCover[curcard->controler]);
		mainGame->btnCardSelect[i]->setRelativePosition(mainGame->Scale<irr::s32>(static_cast<irr::s32>(startpos + i * 125), 55, static_cast<irr::s32>(startpos + 120 + i * 125), 225));
		mainGame->btnCardSelect[i]->setPressed(false);
		mainGame->btnCardSelect[i]->setVisible(true);
		bool design0 = false;
		bool design1 = false;
		for (auto dcode : mainGame->dField.callbg_codes0) {
			if (dcode == curcard->code)
				design0 = true;
		}
		for (auto dcode : mainGame->dField.callbg_codes1) {
			if (dcode == curcard->code)
				design0 = true;
		}
		for (auto dcode : mainGame->dField.crossd_codes) {
			if (dcode == curcard->code)
				design1 = true;
		}
		if (design0 || design1) {
			mainGame->iSelectNegate[i]->setImage(mainGame->imageManager.tNegated);
			mainGame->iSelectNegate[i]->setRelativePosition(mainGame->Scale<irr::s32>(static_cast<irr::s32>(startpos + 10 + i * 125), 90, static_cast<irr::s32>(startpos + 110 + i * 125), 190));
			mainGame->iSelectNegate[i]->setVisible(true);
		}
		else {
			mainGame->iSelectNegate[i]->setVisible(false);
		}
		if(mainGame->dInfo.curMsg != MSG_SORT_CHAIN && mainGame->dInfo.curMsg != MSG_SORT_CARD) {
			sort_list.clear();
			// text
			std::wstring text = L"";
			if(conti_selecting)
				text = std::wstring{ DataManager::unknown_string };
			else if(curcard->location == LOCATION_OVERLAY) {
				text = epro::format(L"{}[{}]({})", gDataManager->FormatLocation(curcard->overlayTarget->location, curcard->overlayTarget->sequence),
					curcard->overlayTarget->sequence + 1, curcard->sequence + 1);
			} else if(curcard->location) {
				text = epro::format(L"{}[{}]", gDataManager->FormatLocation(curcard->location, curcard->sequence),
					curcard->sequence + 1);
			}
			curstring->setText(text.data());
			// color
			if (curcard->is_selected)
				curstring->setBackgroundColor(skin::DUELFIELD_CARD_SELECTED_WINDOW_BACKGROUND_VAL);
			else {
				if(conti_selecting)
					curstring->setBackgroundColor(skin::DUELFIELD_CARD_SELF_WINDOW_BACKGROUND_VAL);
				else if(curcard->location == LOCATION_OVERLAY) {
					if(curcard->owner != curcard->overlayTarget->controler)
						curstring->setOverrideColor(skin::DUELFIELD_CARD_SELECT_WINDOW_OVERLAY_TEXT_VAL);
					if(curcard->overlayTarget->controler)
						curstring->setBackgroundColor(skin::DUELFIELD_CARD_OPPONENT_WINDOW_BACKGROUND_VAL);
					else
						curstring->setBackgroundColor(skin::DUELFIELD_CARD_SELF_WINDOW_BACKGROUND_VAL);
				} else if(curcard->location == LOCATION_DECK || curcard->location == LOCATION_EXTRA || curcard->location == LOCATION_REMOVED) {
					if(curcard->position & POS_FACEDOWN)
						curstring->setOverrideColor(skin::DUELFIELD_CARD_SELECT_WINDOW_SET_TEXT_VAL);
					if(curcard->controler)
						curstring->setBackgroundColor(skin::DUELFIELD_CARD_OPPONENT_WINDOW_BACKGROUND_VAL);
					else
						curstring->setBackgroundColor(skin::DUELFIELD_CARD_SELF_WINDOW_BACKGROUND_VAL);
				} else {
					if(curcard->controler)
						curstring->setBackgroundColor(skin::DUELFIELD_CARD_OPPONENT_WINDOW_BACKGROUND_VAL);
					else
						curstring->setBackgroundColor(skin::DUELFIELD_CARD_SELF_WINDOW_BACKGROUND_VAL);
				}
			}
		} else {
			if(sort_list[i]) {
				curstring->setText(epro::to_wstring(sort_list[i]).data());
			} else
				curstring->setText(L"");
			curstring->setBackgroundColor(skin::DUELFIELD_CARD_SELF_WINDOW_BACKGROUND_VAL);
		}
		curstring->setVisible(true);
		curstring->setRelativePosition(mainGame->Scale<irr::s32>(static_cast<irr::s32>(startpos + i * 125), 30, static_cast<irr::s32>(startpos + 120 + i * 125), 50));
	}
	if(selectable_cards.size() <= 5) {
		for(auto i = selectable_cards.size(); i < 5; ++i) {
			mainGame->btnCardSelect[i]->setVisible(false);
			mainGame->stCardPos[i]->setVisible(false);
			mainGame->iSelectNegate[i]->setVisible(false);
		}
		mainGame->scrCardList->setPos(0);
		mainGame->scrCardList->setVisible(false);
	} else {
		mainGame->scrCardList->setVisible(true);
		mainGame->scrCardList->setMin(0);
		mainGame->scrCardList->setMax(static_cast<irr::s32>(selectable_cards.size() - 5) * 10 + 9);
		mainGame->scrCardList->setPos(0);
	}
	mainGame->btnSelectOK->setVisible(buttonok);
	mainGame->PopupElement(mainGame->wCardSelect);
}
void ClientField::ShowChainCard() {
	sort_list.clear();
	size_t startpos;
	size_t ct;
	if(selectable_cards.size() <= 5) {
		startpos = 30 + 125 * (5 - selectable_cards.size()) / 2;
		ct = selectable_cards.size();
	} else {
		startpos = 30;
		ct = 5;
	}
	for(size_t i = 0; i < ct; ++i) {
		auto& curstring = mainGame->stCardPos[i];
		auto& curcard = selectable_cards[i];
		if(curcard->code)
			mainGame->imageLoading[mainGame->btnCardSelect[i]] = curcard->code;
		else
			mainGame->btnCardSelect[i]->setImage(mainGame->imageManager.tCover[curcard->controler]);
		mainGame->btnCardSelect[i]->setRelativePosition(mainGame->Scale<irr::s32>(static_cast<irr::s32>(startpos + i * 125), 55, static_cast<irr::s32>(startpos + 120 + i * 125), 225));
		mainGame->btnCardSelect[i]->setPressed(false);
		mainGame->btnCardSelect[i]->setVisible(true);
		bool design0 = false;
		bool design1 = false;
		for (auto dcode : mainGame->dField.callbg_codes0) {
			if (dcode == curcard->code)
				design0 = true;
		}
		for (auto dcode : mainGame->dField.callbg_codes1) {
			if (dcode == curcard->code)
				design0 = true;
		}
		for (auto dcode : mainGame->dField.crossd_codes) {
			if (dcode == curcard->code)
				design1 = true;
		}
		if (design0 || design1) {
			mainGame->iSelectNegate[i]->setImage(mainGame->imageManager.tNegated);
			mainGame->iSelectNegate[i]->setRelativePosition(mainGame->Scale<irr::s32>(static_cast<irr::s32>(startpos + i * 125), 55, static_cast<irr::s32>(startpos + 120 + i * 125), 225));
			mainGame->iSelectNegate[i]->setVisible(true);
		}
		else {
			mainGame->iSelectNegate[i]->setVisible(false);
		}
		curstring->setText(epro::format(L"{}[{}]", gDataManager->FormatLocation(curcard->location, curcard->sequence),
			curcard->sequence + 1).data());
		if(curcard->location == LOCATION_OVERLAY) {
			if(curcard->owner != curcard->overlayTarget->controler)
				curstring->setOverrideColor(skin::DUELFIELD_CARD_SELECT_WINDOW_OVERLAY_TEXT_VAL);
			if(curcard->overlayTarget->controler)
				curstring->setBackgroundColor(skin::DUELFIELD_CARD_OPPONENT_WINDOW_BACKGROUND_VAL);
			else
				curstring->setBackgroundColor(skin::DUELFIELD_CARD_SELF_WINDOW_BACKGROUND_VAL);
		} else {
			if(curcard->controler)
				curstring->setBackgroundColor(skin::DUELFIELD_CARD_OPPONENT_WINDOW_BACKGROUND_VAL);
			else
				curstring->setBackgroundColor(skin::DUELFIELD_CARD_SELF_WINDOW_BACKGROUND_VAL);
		}
		curstring->setVisible(true);
		curstring->setRelativePosition(mainGame->Scale<irr::s32>(static_cast<irr::s32>(startpos + i * 125), 30, static_cast<irr::s32>(startpos + 120 + i * 125), 50));
	}
	if(selectable_cards.size() <= 5) {
		for(auto i = selectable_cards.size(); i < 5; ++i) {
			mainGame->btnCardSelect[i]->setVisible(false);
			mainGame->stCardPos[i]->setVisible(false);
			mainGame->iSelectNegate[i]->setVisible(false);
		}
		mainGame->scrCardList->setPos(0);
		mainGame->scrCardList->setVisible(false);
	} else {
		mainGame->scrCardList->setVisible(true);
		mainGame->scrCardList->setMin(0);
		mainGame->scrCardList->setMax(static_cast<irr::s32>(selectable_cards.size() - 5) * 10 + 9);
		mainGame->scrCardList->setPos(0);
	}
	if(!chain_forced)
		mainGame->btnSelectOK->setVisible(true);
	else mainGame->btnSelectOK->setVisible(false);
	mainGame->PopupElement(mainGame->wCardSelect);
}
void ClientField::ShowLocationCard() {
	size_t startpos;
	size_t ct;
	if(display_cards.size() <= 5) {
		startpos = 30 + 125 * (5 - display_cards.size()) / 2;
		ct = display_cards.size();
	} else {
		startpos = 30;
		ct = 5;
	}
	for(size_t i = 0; i < ct; ++i) {
		auto& curstring = mainGame->stDisplayPos[i];
		auto& curcard = display_cards[i];
		curstring->enableOverrideColor(false);
		if(curcard->code)
			mainGame->imageLoading[mainGame->btnCardDisplay[i]] = curcard->code;
		else
			mainGame->btnCardDisplay[i]->setImage(mainGame->imageManager.tCover[curcard->controler]);
		mainGame->btnCardDisplay[i]->setRelativePosition(mainGame->Scale<irr::s32>(static_cast<irr::s32>(startpos + i * 125), 55, static_cast<irr::s32>(startpos + 120 + i * 125), 225));
		mainGame->btnCardDisplay[i]->setPressed(false);
		mainGame->btnCardDisplay[i]->setVisible(true);
		std::wstring text;
		if(curcard->location == LOCATION_OVERLAY) {
			text = epro::format(L"{}[{}]({})", gDataManager->FormatLocation(curcard->overlayTarget->location, curcard->overlayTarget->sequence),
				curcard->overlayTarget->sequence + 1, curcard->sequence + 1);
		} else if(curcard->location) {
			text = epro::format(L"{}[{}]", gDataManager->FormatLocation(curcard->location, curcard->sequence),
				curcard->sequence + 1);
		}
		curstring->setText(text.data());
		if(curcard->location == LOCATION_OVERLAY) {
			if(curcard->owner != curcard->overlayTarget->controler)
				curstring->setOverrideColor(skin::DUELFIELD_CARD_SELECT_WINDOW_OVERLAY_TEXT_VAL);
			if(curcard->overlayTarget->controler)
				curstring->setBackgroundColor(skin::DUELFIELD_CARD_OPPONENT_WINDOW_BACKGROUND_VAL);
			else
				curstring->setBackgroundColor(skin::DUELFIELD_CARD_SELF_WINDOW_BACKGROUND_VAL);
		} else if(curcard->location == LOCATION_EXTRA || curcard->location == LOCATION_REMOVED) {
			if(curcard->position & POS_FACEDOWN)
				curstring->setOverrideColor(skin::DUELFIELD_CARD_SELECT_WINDOW_SET_TEXT_VAL);
			if(curcard->controler)
				curstring->setBackgroundColor(skin::DUELFIELD_CARD_OPPONENT_WINDOW_BACKGROUND_VAL);
			else
				curstring->setBackgroundColor(skin::DUELFIELD_CARD_SELF_WINDOW_BACKGROUND_VAL);
		} else {
			if(curcard->controler)
				curstring->setBackgroundColor(skin::DUELFIELD_CARD_OPPONENT_WINDOW_BACKGROUND_VAL);
			else
				curstring->setBackgroundColor(skin::DUELFIELD_CARD_SELF_WINDOW_BACKGROUND_VAL);
		}
		curstring->setVisible(true);
		curstring->setRelativePosition(mainGame->Scale<irr::s32>(static_cast<irr::s32>(startpos + i * 125), 30, static_cast<irr::s32>(startpos + 120 + i * 125), 50));
	}
	if(display_cards.size() <= 5) {
		for(auto i = display_cards.size(); i < 5; ++i) {
			mainGame->btnCardDisplay[i]->setVisible(false);
			mainGame->stDisplayPos[i]->setVisible(false);
		}
		mainGame->scrDisplayList->setPos(0);
		mainGame->scrDisplayList->setVisible(false);
	} else {
		mainGame->scrDisplayList->setVisible(true);
		mainGame->scrDisplayList->setMin(0);
		mainGame->scrDisplayList->setMax(static_cast<irr::s32>(display_cards.size() - 5) * 10 + 9);
		mainGame->scrDisplayList->setPos(0);
	}
	mainGame->btnDisplayOK->setVisible(true);
	mainGame->PopupElement(mainGame->wCardDisplay);
}
void ClientField::ShowSelectOption(uint64_t select_hint, bool should_lock) {
	std::unique_lock<epro::mutex> lock = (should_lock ? std::unique_lock<epro::mutex>(mainGame->gMutex) : std::unique_lock<epro::mutex>());
	selected_option = 0;
	auto count = select_options.size();
	bool quickmode = true;// (count <= 5);
	for(auto option : select_options) {
		if(mainGame->guiFont->getDimensionustring(gDataManager->GetDesc(option, mainGame->dInfo.compat_mode)).Width > 310) {
			quickmode = false;
			break;
		}
	}
	for(size_t i = 0; (i < count) && (i < 5) && quickmode; i++)
		mainGame->btnOption[i]->setText(gDataManager->GetDesc(select_options[i], mainGame->dInfo.compat_mode).data());
	irr::core::recti pos = mainGame->wOptions->getRelativePosition();
	if(count > 5 && quickmode)
		pos.LowerRightCorner.X = pos.UpperLeftCorner.X + mainGame->Scale(375);
	else
		pos.LowerRightCorner.X = pos.UpperLeftCorner.X + mainGame->Scale(350);
	if(quickmode) {
		mainGame->scrOption->setVisible(count > 5);
		mainGame->scrOption->setMax(static_cast<irr::s32>(count - 5));
		mainGame->scrOption->setMin(0);
		mainGame->scrOption->setPos(0);
		mainGame->stOptions->setVisible(false);
		mainGame->btnOptionp->setVisible(false);
		mainGame->btnOptionn->setVisible(false);
		mainGame->btnOptionOK->setVisible(false);
		for(size_t i = 0; i < 5; i++)
			mainGame->btnOption[i]->setVisible(i < count);
		int newheight = mainGame->Scale(30 + 40 * static_cast<uint8_t>((count > 5) ? 5 : count));
		int oldheight = pos.LowerRightCorner.Y - pos.UpperLeftCorner.Y;
		pos.UpperLeftCorner.Y = pos.UpperLeftCorner.Y + (oldheight - newheight) / 2;
		pos.LowerRightCorner.Y = pos.UpperLeftCorner.Y + newheight;
		mainGame->wOptions->setRelativePosition(pos);
	} else {
		mainGame->stOptions->setText(gDataManager->GetDesc(select_options[0], mainGame->dInfo.compat_mode).data());
		mainGame->stOptions->setVisible(true);
		mainGame->btnOptionp->setVisible(false);
		mainGame->btnOptionn->setVisible(count > 1);
		mainGame->btnOptionOK->setVisible(true);
		for(int i = 0; i < 5; i++)
			mainGame->btnOption[i]->setVisible(false);
		pos.LowerRightCorner.Y = pos.UpperLeftCorner.Y + mainGame->Scale(140);
		mainGame->wOptions->setRelativePosition(pos);
	}
	mainGame->wOptions->setText(gDataManager->GetDesc(select_hint ? select_hint : 555, mainGame->dInfo.compat_mode).data());
	mainGame->PopupElement(mainGame->wOptions);
}
void ClientField::ReplaySwap() {
	auto reset = [](ClientCard* const& pcard)->void {
		if(pcard) {
			pcard->controler = 1 - pcard->controler;
			pcard->UpdateDrawCoordinates(true);
			pcard->is_moving = false;
		}
	};
	auto resetloc = [&reset](const auto& zone)->void {
		for(const auto& pcard : zone)
			reset(pcard);
	};
	std::swap(deck[0], deck[1]);
	std::swap(hand[0], hand[1]);
	std::swap(mzone[0], mzone[1]);
	std::swap(szone[0], szone[1]);
	std::swap(grave[0], grave[1]);
	std::swap(remove[0], remove[1]);
	std::swap(extra[0], extra[1]);
	std::swap(extra_p_count[0], extra_p_count[1]);
	std::swap(skills[0], skills[1]);
	for(int p = 0; p < 2; ++p) {
		resetloc(deck[p]);
		resetloc(hand[p]);
		resetloc(mzone[p]);
		resetloc(szone[p]);
		resetloc(grave[p]);
		resetloc(remove[p]);
		resetloc(extra[p]);
		reset(skills[p]);
	}
	resetloc(overlay_cards);
	mainGame->dInfo.isFirst = !mainGame->dInfo.isFirst;
	mainGame->dInfo.isTeam1 = !mainGame->dInfo.isTeam1;
	mainGame->dInfo.isReplaySwapped = !mainGame->dInfo.isReplaySwapped;
	std::swap(mainGame->dInfo.lp[0], mainGame->dInfo.lp[1]);
	std::swap(mainGame->dInfo.strLP[0], mainGame->dInfo.strLP[1]);
	std::swap(mainGame->dInfo.current_player[0], mainGame->dInfo.current_player[1]);
	std::swap(player_desc_hints[0], player_desc_hints[1]);
	for(auto& chit : chains) {
		chit.controler = 1 - chit.controler;
		chit.UpdateDrawCoordinates();
	}
	disabled_field = (disabled_field >> 16) | (disabled_field << 16);
}
void ClientField::RefreshAllCards() {
	auto refresh = [](ClientCard* const& pcard) {
		if(pcard) {
			pcard->UpdateDrawCoordinates(true);
			pcard->is_moving = false;
			pcard->refresh_on_stop = false;
			pcard->aniFrame = 0;
		}
	};
	auto refreshloc = [&refresh](const auto& zone) {
		for(const auto& pcard : zone)
			refresh(pcard);
	};
	for(int p = 0; p < 2; ++p) {
		refreshloc(deck[p]);
		refreshloc(hand[p]);
		refreshloc(mzone[p]);
		refreshloc(szone[p]);
		refreshloc(grave[p]);
		refreshloc(remove[p]);
		refreshloc(extra[p]);
		refresh(skills[p]);
	}
	refreshloc(overlay_cards);
	for(auto& chit : chains)
		chit.UpdateDrawCoordinates();
	mainGame->should_refresh_hands = true;
}
void ClientField::GetChainDrawCoordinates(uint8_t controler, uint8_t location, uint32_t sequence, irr::core::vector3df* t) {
	if ((location & (~LOCATION_OVERLAY)) == LOCATION_HAND) {
		t->X = 2.95f;
		t->Y = (controler == 0) ? 3.15f : (-3.15f);
		t->Z = 0.03f;
		return;
	}
	auto PileZ = [&](auto& pile) {
		auto multiplier = gGameConfig->topdown_view ? 1 : pile.size();
		t->Z = multiplier * 0.01f + 0.03f;
	};
	const irr::video::S3DVertex* loc = nullptr;
	switch((location & (~LOCATION_OVERLAY))) {
	case LOCATION_DECK: {
		loc = matManager.getDeck()[controler];
		PileZ(deck[controler]);
		break;
	}
	case LOCATION_MZONE: {
		loc = matManager.vFieldMzone[controler][sequence];
		t->Z = 0.03f;
		break;
	}
	case LOCATION_SZONE: {
		loc = matManager.getSzone()[controler][sequence];
		t->Z = 0.03f;
		break;
	}
	case LOCATION_GRAVE: {
		loc = matManager.getGrave()[controler];
		PileZ(grave[controler]);
		break;
	}
	case LOCATION_REMOVED: {
		loc = matManager.getRemove()[controler];
		PileZ(remove[controler]);
		break;
	}
	case LOCATION_EXTRA: {
		loc = matManager.getExtra()[controler];
		PileZ(extra[controler]);
		break;
	}
	default:
		t->X = 0;
		t->Y = 0;
		t->Z = 0;
		return;
	}
	t->X = (loc[0].Pos.X + loc[1].Pos.X) / 2;
	t->Y = (loc[0].Pos.Y + loc[2].Pos.Y) / 2;
}
static void getCardScreenCoordinates(ClientCard* pcard) {
	irr::core::matrix4 trans = mainGame->camera->getProjectionMatrix();
	trans *= mainGame->camera->getViewMatrix();
	trans *= pcard->mTransform;
	auto transform = [&trans, dim = (mainGame->driver->getCurrentRenderTargetSize() / 2)](irr::core::vector3df vec) {
		irr::f32 transformedPos[4] = { vec.X, vec.Y, vec.Z, 1.0f };

		trans.multiplyWith1x4Matrix(transformedPos);

		if(transformedPos[3] < 0)
			return irr::core::vector2d<irr::s32>(-10000, -10000);

		const irr::f32 zDiv = transformedPos[3] == 0.0f ? 1.0f :
			irr::core::reciprocal(transformedPos[3]);

		return irr::core::vector2d<irr::s32>(
			dim.Width + irr::core::round32(dim.Width * (transformedPos[0] * zDiv)),
			dim.Height - irr::core::round32(dim.Height * (transformedPos[1] * zDiv)));
	};

	const auto& frontmat = (pcard->code && (!mainGame->dInfo.isReplay || !gGameConfig->hideHandsInReplays || pcard->is_public || pcard->is_hovered)) ? matManager.vCardFront : matManager.vCardBack;
	const auto upperleft = transform(frontmat[0].Pos);
	const auto lowerright = transform(frontmat[3].Pos);
	auto& collision = pcard->hand_collision;
	collision = { upperleft, lowerright };
	if(!collision.isValid())
		collision.repair();
}
void ClientField::RefreshHandHitboxes() {
	for(const auto& _hand : hand)
		for(const auto& pcard : _hand)
			getCardScreenCoordinates(pcard);
}
void ClientField::GetCardDrawCoordinates(ClientCard* pcard, irr::core::vector3df* t, irr::core::vector3df* r, bool setTrans) {
	const int three_columns = mainGame->dInfo.HasFieldFlag(DUEL_3_COLUMNS_FIELD);
	static const irr::core::vector3df selfATK{ 0.0f, 0.0f, 0.0f };
	static const irr::core::vector3df selfDEF{ 0.0f, 0.0f, -irr::core::HALF_PI };
	static const irr::core::vector3df oppoATK{ 0.0f, 0.0f, irr::core::PI };
	static const irr::core::vector3df oppoDEF{ 0.0f, 0.0f, irr::core::HALF_PI };
	static const irr::core::vector3df facedown{ 0.0f, irr::core::PI, 0.0f };
	static const irr::core::vector3df handfaceup{ -FIELD_ANGLE, 0.0f, 0.0f };
	static const irr::core::vector3df handfacedown{ FIELD_ANGLE, irr::core::PI, 0.0f };
	auto GetMiddleX = [](const Materials::QuadVertex pos)->float {
		return (pos[0].Pos.X + pos[1].Pos.X) / 2.0f;
	};
	auto GetMiddleY = [](const Materials::QuadVertex pos)->float {
		return (pos[0].Pos.Y + pos[2].Pos.Y) / 2.0f;
	};
	if(!pcard->location) return;
	const int& controler = pcard->overlayTarget ? pcard->overlayTarget->controler : pcard->controler;
	const int& sequence = pcard->sequence;
	const int& location = pcard->location;
	const int sqf = mainGame->dInfo.HasFieldFlag(DUEL_SQUARE_FANTASIA);
	auto GetPos = [&]()->const irr::video::S3DVertex* {
		switch(location) {
		case LOCATION_DECK:		return matManager.getDeck()[controler];
		case LOCATION_MZONE:
			if (sqf) {
				if (sequence == 1) {
					const int& atk = pcard->attack;
					if (atk >= MAP_WIDTH * MAP_HEIGHT)
						return matManager.vSomewhere;
					else if (atk < 0)
						return matManager.vSomewhere;
					return matManager.vFieldSquare[controler][atk];
				}
				else
					return matManager.vSomewhere;
			}
			else
				return matManager.vFieldMzone[controler][sequence];
		case LOCATION_SZONE:	return matManager.getSzone()[controler][sequence];
		case LOCATION_GRAVE:	return matManager.getGrave()[controler];
		case LOCATION_REMOVED:	return matManager.getRemove()[controler];
		case LOCATION_EXTRA:	return matManager.getExtra()[controler];
		case LOCATION_SKILL:	return matManager.getSkill()[controler];
		case LOCATION_OVERLAY:
			if(!pcard->overlayTarget || controler > 1)
				return nullptr;
			if (pcard->overlayTarget->location == LOCATION_MZONE) {
				if (sqf) {
					auto ocard = pcard->overlayTarget;
					if ((ocard->sequence == 2) && (pcard->code != 46448938)) {
						if (!ocard->controler) {
							return matManager.vFieldSquare[0][sequence];
						}
						else {
							return matManager.vFieldSquare[0][MAP_WIDTH * MAP_HEIGHT - 1 - sequence];
						}
					}
					return matManager.vSomewhere;
				}
				else
					return matManager.vFieldMzone[controler][pcard->overlayTarget->sequence];
			}
			if(pcard->overlayTarget->location == LOCATION_SZONE)
				return matManager.getSzone()[controler][pcard->overlayTarget->sequence];
			[[fallthrough]];
		default: return nullptr;
		}
	};

	if(location != LOCATION_HAND) {
		const auto pos = GetPos();
		if(!pos)
			return;
		t->X = GetMiddleX(pos);
		t->Y = GetMiddleY(pos);
		t->Z = 0.01f;
		if(location == LOCATION_MZONE) {
			if(controler == 0)
				*r = (pcard->position & POS_DEFENSE) ? selfDEF : selfATK;
			else
				*r = (pcard->position & POS_DEFENSE) ? oppoDEF : oppoATK;
		} else if (location == LOCATION_OVERLAY)
			*r = (pcard->overlayTarget->controler == 0 || sqf) ? selfATK : oppoATK;
		else if (location == LOCATION_SZONE) {
			if (controler == 0)
				*r = !(pcard->position & POS_ATTACK) ? selfDEF : selfATK;
			else
				*r = !(pcard->position & POS_ATTACK) ? oppoDEF : oppoATK;
		}
		else
			*r = (controler == 0) ? selfATK : oppoATK;
		if(((location & (LOCATION_GRAVE | LOCATION_OVERLAY)) == 0) && ((location == LOCATION_DECK && deck_reversed == pcard->is_reversed) ||
			(location != LOCATION_DECK && pcard->position & POS_FACEDOWN))) {
			*r += facedown;
			if(location == LOCATION_MZONE && pcard->position & POS_DEFENSE)
				r->Y = irr::core::PI + 0.001f;
		}
		switch(location) {
			case LOCATION_DECK:
			case LOCATION_GRAVE:
			case LOCATION_REMOVED:
			case LOCATION_EXTRA:
			case LOCATION_SKILL: {
				if(!gGameConfig->topdown_view)
					t->Z += 0.01f * sequence;
				break;
			}
			case LOCATION_OVERLAY: {
				if (!sqf) {
					if (controler == 0)
						*t = { t->X - 0.12f + 0.06f * sequence, t->Y + 0.06f, 0.005f + pcard->sequence * 0.0001f };
					else
						*t = { t->X + 0.12f - 0.06f * sequence, t->Y - 0.06f, 0.005f + pcard->sequence * 0.0001f };
				}
				break;
			}
		}
	} else {
		auto ShouldCardShow = [pcard] {
			return pcard->code && (!mainGame->dInfo.isReplay || !gGameConfig->hideHandsInReplays || pcard->is_public || pcard->is_hovered);
		};
		auto SetHoverState = [&] {
			if(!pcard->is_hovered)
				return;
			if(gGameConfig->topdown_view) {
				if(controler == 0)
					t->Y -= 0.2f;
				else
					t->Y += 0.2f;
				return;
			}
			t->Y -= 0.16f;
			t->Z += 0.656f - 0.5f;
		};
		const auto count = hand[controler].size();
		const size_t max = (6 - gGameConfig->topdown_view - three_columns * 2);
		const float xoff1 = (5.5f - 0.8f * count) / 2.0f + sequence * (gGameConfig->topdown_view ? 0.73f : 0.8f);
		float val = three_columns ? 2.4f : 4.0f;
		if(gGameConfig->topdown_view)
			val -= 0.35f;
		float xoff2 = (sequence * val) / (count - 1);
		if(three_columns) xoff2 += 0.8f;
		auto SetXCoord = [&] {
			if(controler == 0) {
				if(count <= max)
					t->X = 1.55f + xoff1;
				else
					t->X = 1.9f + xoff2;
			} else {
				if(count <= max)
					t->X = 6.25f - xoff1;
				else
					t->X = 5.9f - xoff2;
				if(gGameConfig->topdown_view)
					t->X -= 0.378f;
			}
			if(gGameConfig->topdown_view)
				t->X += 0.3f;
		};
		auto SetYCoord = [&] {
			if(gGameConfig->topdown_view) {
				static constexpr auto base_y = 2.5f;
				if(controler == 0)
					t->Y = base_y;
				else
					t->Y = base_y * -1.0f;
				return;
			}
			if(controler == 0)
				t->Y = 4.0f;
			else
				t->Y = -3.4f;

		};
		const float zoff1 = gGameConfig->topdown_view ? 3.0f : 0.5f;
		const float zoff2 = (controler == 0) ? (0.001f * sequence) : (-0.001f * sequence);
		SetXCoord();
		SetYCoord();
		t->Z = zoff1 + zoff2;
		SetHoverState();
		if(gGameConfig->topdown_view) {
			if(controler == 0)
				*r = selfATK;
			else
				*r = oppoATK;
		}
		if(!ShouldCardShow()) {
			if(gGameConfig->topdown_view)
				*r += facedown;
			else
				*r = handfacedown;
		} else if(!gGameConfig->topdown_view)
			*r = handfaceup;
	}
	if(setTrans) {
		pcard->mTransform.setTranslation(*t);
		pcard->mTransform.setRotationRadians(*r);
		if(pcard->location == LOCATION_HAND && !pcard->is_hovered)
			getCardScreenCoordinates(pcard);
	}
}
void ClientField::MoveCard(ClientCard* pcard, float frame) {
	float milliseconds = frame * 1000.0f / 60.0f;
	irr::core::vector3df trans = pcard->curPos;
	irr::core::vector3df rot = pcard->curRot;
	GetCardDrawCoordinates(pcard, &trans, &rot);
	pcard->dPos = (trans - pcard->curPos) / milliseconds;
	float diff = rot.X - pcard->curRot.X;
	while (diff < 0) diff += irr::core::PI * 2;
	while (diff > irr::core::PI * 2)
		diff -= irr::core::PI * 2;
	if (diff < irr::core::PI)
		pcard->dRot.X = diff / milliseconds;
	else
		pcard->dRot.X = -(irr::core::PI * 2 - diff) / milliseconds;
	diff = rot.Y - pcard->curRot.Y;
	while (diff < 0) diff += irr::core::PI * 2;
	while (diff > irr::core::PI * 2) diff -= irr::core::PI * 2;
	if (diff < irr::core::PI)
		pcard->dRot.Y = diff / milliseconds;
	else
		pcard->dRot.Y = -(irr::core::PI * 2 - diff) / milliseconds;
	diff = rot.Z - pcard->curRot.Z;
	while (diff < 0) diff += irr::core::PI * 2;
	while (diff > irr::core::PI * 2) diff -= irr::core::PI * 2;
	if (diff < irr::core::PI)
		pcard->dRot.Z = diff / milliseconds;
	else
		pcard->dRot.Z = -(irr::core::PI * 2 - diff) / milliseconds;
	pcard->is_moving = true;
	pcard->refresh_on_stop = true;
	pcard->aniFrame = milliseconds;
}
void ClientField::FadeCard(ClientCard* pcard, float alpha, float frame) {
	float milliseconds = frame * 1000.0f / 60.0f;
	pcard->dAlpha = (alpha - pcard->curAlpha) / milliseconds;
	pcard->is_fading = true;
	pcard->aniFrame = milliseconds;
}
bool ClientField::ShowSelectSum() {
	if(CheckSelectSum()) {
		if(selectsum_cards.size() == 0 || selectable_cards.size() == 0) {
			SetResponseSelectedCards();
			ShowCancelOrFinishButton(0);
			DuelClient::SendResponse();
			return true;
		} else {
			select_ready = true;
		}
	} else
		select_ready = false;
	bool panelmode = false;
	for (auto& card : selectable_cards) {
		if (card->location & (LOCATION_DECK+LOCATION_EXTRA+LOCATION_GRAVE+LOCATION_REMOVED+LOCATION_OVERLAY)) {
			panelmode = true;
			break;
		}
	}
	mainGame->wCardSelect->setVisible(false);
	mainGame->stCardListTip->setVisible(false);
	if(panelmode) {
		mainGame->dField.ShowSelectCard(select_ready);
	}
	mainGame->stHintMsg->setVisible(!panelmode);
	if (select_ready) {
		ShowCancelOrFinishButton(2);
	} else {
		ShowCancelOrFinishButton(0);
	}
	return false;
}
bool ClientField::CheckSelectSum() {
	std::set<ClientCard*> selable;
	for(auto& card : selectsum_all) {
		card->is_selectable = false;
		card->is_selected = false;
		selable.insert(card);
	}
	for(auto& card : must_select_cards) {
		card->is_selectable = true;
		card->is_selected = true;
		selable.erase(card);
	}
	for(auto& card : selected_cards) {
		card->is_selectable = true;
		card->is_selected = true;
		selable.erase(card);
	}
	selected_cards.insert(selected_cards.end(), must_select_cards.begin(), must_select_cards.end());
	selectsum_cards.clear();
	for(auto& card : selectable_cards) {
		SetShowMark(card, false);
	}
	mainGame->stCardListTip->setVisible(false);
	if (select_mode == 0) {
		bool ret = check_sel_sum_s(selable, 0, select_sumval);
		selectable_cards.clear();
		std::sort(mainGame->dField.must_select_cards.begin(), mainGame->dField.must_select_cards.end(), ClientCard::client_card_sort);
		for(auto& card : must_select_cards) {
			card->is_selectable = true;
			selectable_cards.push_back(card);
			auto it = std::find(selected_cards.begin(), selected_cards.end(), card);
			if (it != selected_cards.end())
				selected_cards.erase(it);
		}
		std::sort(mainGame->dField.selected_cards.begin(), mainGame->dField.selected_cards.end(), ClientCard::client_card_sort);
		for(auto& card : selected_cards) {
			card->is_selectable = true;
			selectable_cards.push_back(card);
		}
		std::vector<ClientCard*> tmp(selectsum_cards.begin(), selectsum_cards.end());
		std::sort(tmp.begin(), tmp.end(), ClientCard::client_card_sort);
		for(auto& card : tmp) {
			card->is_selectable = true;
			selectable_cards.push_back(card);
		}
		return ret;
	} else {
		int mm = -1, mx = -1;
		uint32_t max = 0, sumc = 0;
		bool ret = false;
		for (auto sit = selected_cards.begin(); sit != selected_cards.end(); ++sit) {
			int op1 = (*sit)->opParam & 0xffff;
			int op2 = (*sit)->opParam >> 16;
			int opmin = (op2 > 0 && op1 > op2) ? op2 : op1;
			int opmax = op2 > op1 ? op2 : op1;
			if (mm == -1 || opmin < mm)
				mm = opmin;
			if (mx == -1 || opmax < mx)
				mx = opmax;
			sumc += opmin;
			max += opmax;
		}
		if (select_sumval <= sumc) {
			for (auto& card : must_select_cards) {
				auto it = std::find(selected_cards.begin(), selected_cards.end(), card);
				if (it != selected_cards.end())
					selected_cards.erase(it);
			}
			return true;
		}
		if (select_sumval <= max && select_sumval > max - mx)
			ret = true;
		for(auto sit = selable.begin(); sit != selable.end(); ++sit) {
			uint16_t op1 = (*sit)->opParam & 0xffff;
			uint16_t op2 = ((*sit)->opParam >> 16) & 0xffff;
			uint16_t m = op1;
			uint32_t sums = sumc;
			sums += m;
			int ms = mm;
			if (ms == -1 || m < ms)
				ms = m;
			if (sums >= select_sumval) {
				if (sums - ms < select_sumval)
					selectsum_cards.insert(*sit);
			} else {
				std::set<ClientCard*> left(selable);
				left.erase(*sit);
				if (check_min(left, left.begin(), select_sumval - sums, select_sumval - sums + ms - 1))
					selectsum_cards.insert(*sit);
			}
			if (op2 == 0)
				continue;
			m = op2;
			sums = sumc;
			sums += m;
			ms = mm;
			if (ms == -1 || m < ms)
				ms = m;
			if (sums >= select_sumval) {
				if (sums - ms < select_sumval)
					selectsum_cards.insert(*sit);
			} else {
				std::set<ClientCard*> left(selable);
				left.erase(*sit);
				if (check_min(left, left.begin(), select_sumval - sums, select_sumval - sums + ms - 1))
					selectsum_cards.insert(*sit);
			}
		}
		selectable_cards.clear();
		std::sort(must_select_cards.begin(), must_select_cards.end(), ClientCard::client_card_sort);
		for(auto& card : must_select_cards) {
			card->is_selectable = true;
			selectable_cards.push_back(card);
			auto it = std::find(selected_cards.begin(), selected_cards.end(), card);
			if (it != selected_cards.end())
				selected_cards.erase(it);
		}
		std::sort(selected_cards.begin(), selected_cards.end(), ClientCard::client_card_sort);
		for(auto& card : selected_cards) {
			card->is_selectable = true;
			selectable_cards.push_back(card);
		}
		std::vector<ClientCard*> tmp(selectsum_cards.begin(), selectsum_cards.end());
		std::sort(tmp.begin(), tmp.end(), ClientCard::client_card_sort);
		for(auto& card : tmp) {
			card->is_selectable = true;
			selectable_cards.push_back(card);
		}
		return ret;
	}
}
void ClientField::ShowSelectRace(uint64_t race) {
	uint64_t filter = 0x1;
	auto selected = 0;
	for(auto i = 0u; i < sizeofarr(mainGame->chkRace); ++i, filter <<= 1) {
		auto* checkBox = mainGame->chkRace[i];
		checkBox->setChecked(false);
		auto checked = (filter & race) != 0;
		checkBox->setVisible(checked);
		if(checked) {
			checkBox->setRelativePosition(mainGame->Scale<irr::s32>(10 + (selected % 3) * 120, (selected / 3) * 25, 150 + (selected % 3) * 120, 25 + (selected / 3) * 25));
			++selected;
		}
	}
}
bool ClientField::check_min(const std::set<ClientCard*>& left, std::set<ClientCard*>::const_iterator index, int min, int max) {
	if (index == left.end())
		return false;
	int op1 = (*index)->opParam & 0xffff;
	int op2 = (*index)->opParam >> 16;
	int m = (op2 > 0 && op1 > op2) ? op2 : op1;
	if (m >= min && m <= max)
		return true;
	++index;
	return (min > m && check_min(left, index, min - m, max - m))
	        || check_min(left, index, min, max);
}
bool ClientField::check_sel_sum_s(const std::set<ClientCard*>& left, size_t index, int acc) {
	if (acc < 0)
		return false;
	if (index == selected_cards.size()) {
		if (acc == 0) {
			uint32_t count = static_cast<uint32_t>(selected_cards.size()) - must_select_count;
			return count >= select_min && count <= select_max;
		}
		check_sel_sum_t(left, acc);
		return false;
	}
	auto l = selected_cards[index]->opParam;
	int l1 = l & 0xffff;
	int l2 = l >> 16;
	bool res1 = false, res2 = false;
	res1 = check_sel_sum_s(left, index + 1, acc - l1);
	if (l2 > 0)
		res2 = check_sel_sum_s(left, index + 1, acc - l2);
	return res1 || res2;
}
void ClientField::check_sel_sum_t(const std::set<ClientCard*>& left, int acc) {
	uint32_t count = static_cast<uint32_t>(selected_cards.size()) + 1 - must_select_count;
	for (auto sit = left.begin(); sit != left.end(); ++sit) {
		if (selectsum_cards.find(*sit) != selectsum_cards.end())
			continue;
		std::set<ClientCard*> testlist(left);
		testlist.erase(*sit);
		auto l = (*sit)->opParam;
		int l1 = l & 0xffff;
		int l2 = l >> 16;
		if (check_sum(testlist.begin(), testlist.end(), acc - l1, count)
		        || (l2 > 0 && check_sum(testlist.begin(), testlist.end(), acc - l2, count))) {
			selectsum_cards.insert(*sit);
		}
	}
}
bool ClientField::check_sum(std::set<ClientCard*>::const_iterator index, std::set<ClientCard*>::const_iterator end, int acc, uint32_t count) {
	if (acc == 0)
		return count >= select_min && count <= select_max;
	if (acc < 0 || index == end)
		return false;
	int l = (*index)->opParam;
	int l1 = l & 0xffff;
	int l2 = l >> 16;
	if ((l1 == acc || (l2 > 0 && l2 == acc)) && (count + 1 >= select_min) && (count + 1 <= select_max))
		return true;
	++index;
	return (acc > l1 && check_sum(index, end, acc - l1, count + 1))
	       || (l2 > 0 && acc > l2 && check_sum(index, end, acc - l2, count + 1))
	       || check_sum(index, end, acc, count);
}
#define BINARY_OP(opcode,op) case opcode: {\
								if (stack.size() >= 2) {\
									auto rhs = stack.top();\
									stack.pop();\
									auto lhs = stack.top();\
									stack.pop();\
									stack.push(lhs op rhs);\
								}\
								break;\
							}
#define UNARY_OP(opcode,op) case opcode: {\
								if (stack.size() >= 1) {\
									auto val = stack.top();\
									stack.pop();\
									stack.push(op val);\
								}\
								break;\
							}
#define UNARY_OP_OP(opcode,val,op) UNARY_OP(opcode,cd->val op)
#define GET_OP(opcode,val) case opcode: {\
								stack.push(cd->val);\
								break;\
							}
static bool is_declarable(const CardDataC* cd, const std::vector<uint64_t>& opcodes) {
	std::stack<int64_t> stack;
	bool alias = false, token = false;
	for(auto& opcode : opcodes) {
		switch(opcode << (mainGame->dInfo.compat_mode ? 32 : 0)) {
		BINARY_OP(OPCODE_ADD, +);
		BINARY_OP(OPCODE_SUB, -);
		BINARY_OP(OPCODE_MUL, *);
		BINARY_OP(OPCODE_DIV, /);
		BINARY_OP(OPCODE_AND, &&);
		BINARY_OP(OPCODE_OR, ||);
		UNARY_OP(OPCODE_NEG, -);
		UNARY_OP(OPCODE_NOT, !);
		BINARY_OP(OPCODE_BAND, &);
		BINARY_OP(OPCODE_BOR, |);
		UNARY_OP(OPCODE_BNOT, ~);
		BINARY_OP(OPCODE_BXOR, ^);
		BINARY_OP(OPCODE_LSHIFT, <<);
		BINARY_OP(OPCODE_RSHIFT, >>);
		UNARY_OP_OP(OPCODE_ISCODE, code, ==);
		UNARY_OP_OP(OPCODE_ISTYPE, type, &);
		UNARY_OP_OP(OPCODE_ISRACE, race, &);
		UNARY_OP_OP(OPCODE_ISATTRIBUTE, attribute, &);
		GET_OP(OPCODE_GETCODE, code);
		GET_OP(OPCODE_GETTYPE, type);
		GET_OP(OPCODE_GETRACE, race);
		GET_OP(OPCODE_GETATTRIBUTE, attribute);
		//GET_OP(OPCODE_GETSETCARD, setcode);
		case OPCODE_ISSETCARD: {
			if (stack.size() >= 1) {
				int set_code = stack.top();
				stack.pop();
				bool res = false;
				uint16_t settype = set_code & 0xfff;
				uint16_t setsubtype = set_code & 0xf000;
				for(auto& sc : cd->setcodes) {
					if((sc & 0xfff) == settype && (sc & 0xf000 & setsubtype) == setsubtype) {
						res = true;
						break;
					}
				}
				stack.push(res);
			}
			break;
		}
		case OPCODE_ALLOW_ALIASES: {
			alias = true;
			break;
		}
		case OPCODE_ALLOW_TOKENS: {
			token = true;
			break;
		}
		default: {
			stack.push(opcode);
			break;
		}
		}
	}
	if(stack.size() != 1 || stack.top() == 0)
		return false;
	return cd->code == CARD_MARINE_DOLPHIN || cd->code == CARD_TWINKLE_MOSS
		|| ((alias || !cd->alias) && (token || ((cd->type & (TYPE_MONSTER + TYPE_TOKEN)) != (TYPE_MONSTER + TYPE_TOKEN))));
}
#undef BINARY_OP
#undef UNARY_OP
#undef UNARY_OP_OP
#undef GET_OP
void ClientField::UpdateDeclarableList(bool refresh) {
	CardDataM* cd = nullptr;
	auto check_code = [&, cards_end = gDataManager->cards.end()](uint32_t trycode) -> bool {
		const auto it = gDataManager->cards.find(trycode);
		cd = nullptr;
		if(it != cards_end && is_declarable(&it->second._data, declare_opcodes))
			cd = &it->second;
		return cd;
	};
	auto ptext = mainGame->ebANCard->getText();
	if(ptext[0] == 0 && !refresh) {
		std::vector<uint32_t> cache;
		cache.swap(ancard);
		int sel = mainGame->lstANCard->getSelected();
		uint32_t selcode = (sel == -1) ? 0 : cache[sel];
		mainGame->lstANCard->clear();
		for(const auto& trycode : cache) {
			if(check_code(trycode)) {
				ancard.push_back(trycode);
				auto idx = mainGame->lstANCard->addItem(cd->GetStrings().name.data());
				if(trycode == selcode)
					mainGame->lstANCard->setSelected(idx);
			}
		}
		if(ancard.size() > 0)
			return;
	}
	if(check_code(BufferIO::GetVal(ptext))) {
		mainGame->lstANCard->clear();
		mainGame->lstANCard->addItem(cd->GetStrings().name.data());
		ancard = { cd->_data.code };
		return;
	}
	const auto pname = Utils::ToUpperNoAccents<std::wstring>(ptext);
	mainGame->lstANCard->clear();
	ancard.clear();
	for(const auto& card : gDataManager->cards) {
		const auto& strings = card.second.GetStrings();
		const auto& name = strings.uppercase_name;
		if(name.find(pname) != std::wstring::npos) {
			if(is_declarable(&card.second._data, declare_opcodes)) {
				if(pname == name) { //exact match
					mainGame->lstANCard->insertItem(0, strings.name.data(), -1);
					ancard.insert(ancard.begin(), card.first);
				} else {
					mainGame->lstANCard->addItem(strings.name.data());
					ancard.push_back(card.first);
				}
			}
		}
	}
}
void ChainInfo::UpdateDrawCoordinates() {
	mainGame->dField.GetChainDrawCoordinates(controler, location, sequence, &chain_pos);
}
}
