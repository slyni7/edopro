#include "materials.h"
#include "custom_skin_enum.h"

/*
in irrlicht 1.9, ONETEXTURE_BLEND materials are considered as transparent
where in 1.8.4 they're not and since they're considered transparent, the depth
buffer is ignored when drawing them, thus causing xyz materials to be drawn
in the wrong order
*/
#if (IRRLICHT_VERSION_MAJOR==1 && IRRLICHT_VERSION_MINOR==9)
#define ENABLE_ZWRITE(mat) do {mat.ZWriteEnable = irr::video::EZW_ON; } while(0)
#else
#define ENABLE_ZWRITE(mat) (void)0
#endif

namespace ygo {

Materials matManager;

inline void SetS3DVertex(Materials::QuadVertex v, irr::f32 x1, irr::f32 y1, irr::f32 x2, irr::f32 y2, irr::f32 z, irr::f32 nz, irr::f32 tu1, irr::f32 tv1, irr::f32 tu2, irr::f32 tv2) {
	v[0] = irr::video::S3DVertex(x1, y1, z, 0, 0, nz, irr::video::SColor(255, 255, 255, 255), tu1, tv1);
	v[1] = irr::video::S3DVertex(x2, y1, z, 0, 0, nz, irr::video::SColor(255, 255, 255, 255), tu2, tv1);
	v[2] = irr::video::S3DVertex(x1, y2, z, 0, 0, nz, irr::video::SColor(255, 255, 255, 255), tu1, tv2);
	v[3] = irr::video::S3DVertex(x2, y2, z, 0, 0, nz, irr::video::SColor(255, 255, 255, 255), tu2, tv2);
}

Materials::Materials() {
	SetS3DVertex(vCardCenter, -0.385f, -0.55f, 0.385f, 0.55f, 0, 1, 0, 0, 1, 1);
	SetS3DVertex(vCardFront, -0.35f, -0.5f, 0.35f, 0.5f, 0, 1, 0, 0, 1, 1);
	SetS3DVertex(vCardOutline, -0.375f, -0.54f, 0.37f, 0.54f, 0, 1, 0, 0, 1, 1);
	SetS3DVertex(vCardOutliner, 0.37f, -0.54f, -0.375f, 0.54f, 0, 1, 0, 0, 1, 1);
	SetS3DVertex(vCardBack, 0.35f, -0.5f, -0.35f, 0.5f, 0, -1, 0, 0, 1, 1);
	SetS3DVertex(vSquareFront, -0.35f * 5 / MAP_WIDTH, -0.5f * 5 / MAP_HEIGHT, 0.35f * 5 / MAP_WIDTH, 0.5f * 5 / MAP_HEIGHT, 0, 1, 0, 0, 1, 1);
	SetS3DVertex(vSymbol, -0.35f, -0.35f, 0.35f, 0.35f, 0.01f, 1, 0, 0, 1, 1);
	SetS3DVertex(vNegate, -0.25f, -0.28f, 0.25f, 0.22f, 0.01f, 1, 0, 0, 1, 1);
	SetS3DVertex(vChainNum, -0.35f, -0.35f, 0.35f, 0.35f, 0, 1, 0, 0, 0.19375f, 0.2421875f);
	SetS3DVertex(vActivate, -0.5f, -0.5f, 0.5f, 0.5f, 0, 1, 0, 0, 1, 1);
	SetS3DVertex(vField, -1.0f, -4.0f, 9.0f, 4.0f, 0, 1, 0, 0, 1, 1);
	SetS3DVertex(vFieldSpell[0], 1.2f, -3.2f, 6.7f, 3.2f, 0, 1, 0, 0, 1, 1);
	SetS3DVertex(vFieldSpell[1], 2.3f, -3.2f, 5.6f, 3.2f, 0, 1, 0, 0, 1, 1);
	SetS3DVertex(vFieldSpell1[0], 1.2f, 0.8f, 6.7f, 3.2f, 0, 1, 0, 0.2f, 1, 0.63636f);
	SetS3DVertex(vFieldSpell1[1], 2.3f, 0.8f, 5.6f, 3.2f, 0, 1, 0, 0.2f, 1, 0.63636f);
	SetS3DVertex(vFieldSpell2[0], 1.2f, -3.2f, 6.7f, -0.8f, 0, 1, 1, 0.63636f, 0, 0.2f);
	SetS3DVertex(vFieldSpell2[1], 2.3f, -3.2f, 5.6f, -0.8f, 0, 1, 1, 0.63636f, 0, 0.2f);

	iRectangle[0] = 0;
	iRectangle[1] = 1;
	iRectangle[2] = 2;
	iRectangle[3] = 2;
	iRectangle[4] = 1;
	iRectangle[5] = 3;

	SetS3DVertex(vSomewhere, 19.9f, 19.9f, 20.7f, 21.1f, 0, 1, 0, 0, 0, 0);

	SetS3DVertex(vFieldDeck[0][0], 6.9f, 2.7f, 7.7f, 3.9f, 0, 1, 0, 0, 0, 0);
	SetS3DVertex(vFieldDeck[1][0], 5.8f, 2.7f, 6.6f, 3.9f, 0, 1, 0, 0, 0, 0);
	//grave
	SetS3DVertex(vFieldGrave[0][0][0], 6.9f, 0.1f, 7.7f, 1.3f, 0, 1, 0, 0, 0, 0);
	SetS3DVertex(vFieldGrave[1][0][0], 6.9f, 1.4f, 7.7f, 2.6f, 0, 1, 0, 0, 0, 0);
	//speed duel grave
	SetS3DVertex(vFieldGrave[0][1][0], 5.8f, 0.1f, 6.6f, 1.3f, 0, 1, 0, 0, 0, 0);
	SetS3DVertex(vFieldGrave[1][1][0], 5.8f, 1.4f, 6.6f, 2.6f, 0, 1, 0, 0, 0, 0);
	//extra
	SetS3DVertex(vFieldExtra[0][0], 0.2f, 2.7f, 1.0f, 3.9f, 0, 1, 0, 0, 0, 0);
	SetS3DVertex(vFieldExtra[1][0], 1.3f, 2.7f, 2.1f, 3.9f, 0, 1, 0, 0, 0, 0);
	//remove
	SetS3DVertex(vFieldRemove[0][0][0], 7.9f, 0.1f, 8.7f, 1.3f, 0, 1, 0, 0, 0, 0);
	SetS3DVertex(vFieldRemove[1][0][0], 6.9f, 0.1f, 7.7f, 1.3f, 0, 1, 0, 0, 0, 0);
	//speed duel remove
	SetS3DVertex(vFieldRemove[0][1][0], 6.8f, 0.1f, 7.6f, 1.3f, 0, 1, 0, 0, 0, 0);
	SetS3DVertex(vFieldRemove[1][1][0], 5.8f, 0.1f, 6.6f, 1.3f, 0, 1, 0, 0, 0, 0);
	for (int i = 0; i < MAP_WIDTH; ++i) {
		for (int j = 0; j < MAP_HEIGHT; ++j) {
			SetS3DVertex(vFieldSquare[0][i + j * MAP_HEIGHT], 1.2f + i * (5.5f / MAP_WIDTH), 3.2f - (j + 1) * (6.4f / MAP_HEIGHT), 1.2f + (i + 1) * (5.5f / MAP_WIDTH), 3.2f - j * (6.4f / MAP_HEIGHT), 0, 1, 0, 0, 0, 0);
			SetS3DVertex(vFieldSquare[1][i + j * MAP_HEIGHT], 6.7f - i * (5.5f / MAP_WIDTH), -3.2f + (j + 1) * (6.4f / MAP_HEIGHT), 6.7f - (i + 1) * (5.5f / MAP_WIDTH), -3.2f + j * (6.4f / MAP_HEIGHT), 0, 1, 0, 0, 0, 0);
		}
	}
	for(int i = 0; i < 5; ++i)
		SetS3DVertex(vFieldMzone[0][i], 1.2f + i * 1.1f, 0.8f, 2.3f + i * 1.1f, 2.0f, 0, 1, 0, 0, 0, 0);
	SetS3DVertex(vFieldMzone[0][5], 2.3f, -0.6f, 3.4f, 0.6f, 0, 1, 0, 0, 0, 0);
	SetS3DVertex(vFieldMzone[0][6], 4.5f, -0.6f, 5.6f, 0.6f, 0, 1, 0, 0, 0, 0);
	for (int i = 0; i < 5; ++i) {
		SetS3DVertex(vFieldSzone[0][0][0][i], 1.2f + i * 1.1f, 2.0f, 2.3f + i * 1.1f, 3.2f, 0, 1, 0, 0, 0, 0);
		SetS3DVertex(vFieldSzone[1][0][0][i], 1.2f + i * 1.1f, 2.0f, 2.3f + i * 1.1f, 3.2f, 0, 1, 0, 0, 0, 0);
		SetS3DVertex(vFieldSzone[0][1][0][i], 1.2f + i * 1.1f, 2.0f, 2.3f + i * 1.1f, 3.2f, 0, 1, 0, 0, 0, 0);
		SetS3DVertex(vFieldSzone[1][1][0][i], 1.2f + i * 1.1f, 2.0f, 2.3f + i * 1.1f, 3.2f, 0, 1, 0, 0, 0, 0);
	}
	//field
	SetS3DVertex(vFieldSzone[0][0][0][5], 0.2f, 0.1f, 1.0f, 1.3f, 0, 1, 0, 0, 0, 0);
	SetS3DVertex(vFieldSzone[1][0][0][5], 0.2f, 1.4f, 1.0f, 2.6f, 0, 1, 0, 0, 0, 0);
	//field speed duel
	SetS3DVertex(vFieldSzone[0][1][0][5], 1.3f, 0.1f, 2.1f, 1.3f, 0, 1, 0, 0, 0, 0);
	SetS3DVertex(vFieldSzone[1][1][0][5], 1.3f, 1.4f, 2.1f, 2.6f, 0, 1, 0, 0, 0, 0);
	//LScale
	SetS3DVertex(vFieldSzone[0][0][0][6], 0.2f, 1.4f, 1.0f, 2.6f, 0, 1, 0, 0, 0, 0);
	SetS3DVertex(vFieldSzone[1][0][0][6], 0.2f, 0.1f, 1.0f, 1.3f, 0, 1, 0, 0, 0, 0);
	//LScale speed duel
	SetS3DVertex(vFieldSzone[0][1][0][6], 1.3f, 1.4f, 2.1f, 2.6f, 0, 1, 0, 0, 0, 0);
	SetS3DVertex(vFieldSzone[1][1][0][6], 1.3f, 0.1f, 2.1f, 1.3f, 0, 1, 0, 0, 0, 0);
	//RScale
	SetS3DVertex(vFieldSzone[0][0][0][7], 6.9f, 1.4f, 7.7f, 2.6f, 0, 1, 0, 0, 0, 0);
	SetS3DVertex(vFieldSzone[1][0][0][7], 7.9f, 0.1f, 8.7f, 1.3f, 0, 1, 0, 0, 0, 0);
	//RScale speed duel
	SetS3DVertex(vFieldSzone[0][1][0][7], 5.8f, 1.4f, 6.6f, 2.6f, 0, 1, 0, 0, 0, 0);
	SetS3DVertex(vFieldSzone[1][1][0][7], 6.8f, 0.1f, 7.6f, 1.3f, 0, 1, 0, 0, 0, 0);

	SetS3DVertex(vFieldDeck[0][1], 1.0f, -2.7f, 0.2f, -3.9f, 0, 1, 0, 0, 0, 0);
	SetS3DVertex(vFieldDeck[1][1], 2.1f, -2.7f, 1.3f, -3.9f, 0, 1, 0, 0, 0, 0);
	//grave
	SetS3DVertex(vFieldGrave[0][0][1], 1.0f, -0.1f, 0.2f, -1.3f, 0, 1, 0, 0, 0, 0);
	SetS3DVertex(vFieldGrave[1][0][1], 1.0f, -1.4f, 0.2f, -2.6f, 0, 1, 0, 0, 0, 0);
	//speed duel grave
	SetS3DVertex(vFieldGrave[0][1][1], 2.1f, -0.1f, 1.3f, -1.3f, 0, 1, 0, 0, 0, 0);
	SetS3DVertex(vFieldGrave[1][1][1], 2.1f, -1.4f, 1.3f, -2.6f, 0, 1, 0, 0, 0, 0);
	//extra
	SetS3DVertex(vFieldExtra[0][1], 7.7f, -2.7f, 6.9f, -3.9f, 0, 1, 0, 0, 0, 0);
	SetS3DVertex(vFieldExtra[1][1], 6.6f, -2.7f, 5.8f, -3.9f, 0, 1, 0, 0, 0, 0);
	//remove
	SetS3DVertex(vFieldRemove[0][0][1], 0.0f, -0.1f, -0.8f, -1.3f, 0, 1, 0, 0, 0, 0);
	SetS3DVertex(vFieldRemove[1][0][1], 1.0f, -0.1f, 0.2f, -1.3f, 0, 1, 0, 0, 0, 0);
	//speed duel remove
	SetS3DVertex(vFieldRemove[0][1][1], 1.1f, -0.1f, 0.3f, -1.3f, 0, 1, 0, 0, 0, 0);
	SetS3DVertex(vFieldRemove[1][1][1], 2.1f, -0.1f, 1.3f, -1.3f, 0, 1, 0, 0, 0, 0);
	for(int i = 0; i < 5; ++i)
		SetS3DVertex(vFieldMzone[1][i], 6.7f - i * 1.1f, -0.8f, 5.6f - i * 1.1f, -2.0f, 0, 1, 0, 0, 0, 0);
	SetS3DVertex(vFieldMzone[1][5], 5.6f, 0.6f, 4.5f, -0.6f, 0, 1, 0, 0, 0, 0);
	SetS3DVertex(vFieldMzone[1][6], 3.4f, 0.6f, 2.3f, -0.6f, 0, 1, 0, 0, 0, 0);
	for (int i = 0; i < 5; ++i) {
		SetS3DVertex(vFieldSzone[0][0][1][i], 6.7f - i * 1.1f, -2.0f, 5.6f - i * 1.1f, -3.2f, 0, 1, 0, 0, 0, 0);
		SetS3DVertex(vFieldSzone[1][0][1][i], 6.7f - i * 1.1f, -2.0f, 5.6f - i * 1.1f, -3.2f, 0, 1, 0, 0, 0, 0);
		SetS3DVertex(vFieldSzone[0][1][1][i], 6.7f - i * 1.1f, -2.0f, 5.6f - i * 1.1f, -3.2f, 0, 1, 0, 0, 0, 0);
		SetS3DVertex(vFieldSzone[1][1][1][i], 6.7f - i * 1.1f, -2.0f, 5.6f - i * 1.1f, -3.2f, 0, 1, 0, 0, 0, 0);
	}
	//field
	SetS3DVertex(vFieldSzone[0][0][1][5], 7.7f, -0.1f, 6.9f, -1.3f, 0, 1, 0, 0, 0, 0);
	SetS3DVertex(vFieldSzone[1][0][1][5], 7.7f, -1.4f, 6.9f, -2.6f, 0, 1, 0, 0, 0, 0);
	//field speed duel
	SetS3DVertex(vFieldSzone[0][1][1][5], 6.6f, -0.1f, 5.8f, -1.3f, 0, 1, 0, 0, 0, 0);
	SetS3DVertex(vFieldSzone[1][1][1][5], 6.6f, -1.4f, 5.8f, -2.6f, 0, 1, 0, 0, 0, 0);
	//LScale
	SetS3DVertex(vFieldSzone[0][0][1][6], 7.7f, -1.4f, 6.9f, -2.6f, 0, 1, 0, 0, 0, 0);
	SetS3DVertex(vFieldSzone[1][0][1][6], 7.7f, -0.1f, 6.9f, -1.3f, 0, 1, 0, 0, 0, 0);
	//LScale speed duel
	SetS3DVertex(vFieldSzone[0][1][1][6], 6.6f, -1.4f, 5.8f, -2.6f, 0, 1, 0, 0, 0, 0);
	SetS3DVertex(vFieldSzone[1][1][1][6], 6.6f, -0.1f, 5.8f, -1.3f, 0, 1, 0, 0, 0, 0);
	//RScale
	SetS3DVertex(vFieldSzone[0][0][1][7], 1.0f, -1.4f, 0.2f, -2.6f, 0, 1, 0, 0, 0, 0);
	SetS3DVertex(vFieldSzone[1][0][1][7], 0.0f, -0.1f, -0.8f, -1.3f, 0, 1, 0, 0, 0, 0);
	//RScale speed duel
	SetS3DVertex(vFieldSzone[0][1][1][7], 2.1f, -1.4f, 1.3f, -2.6f, 0, 1, 0, 0, 0, 0);
	SetS3DVertex(vFieldSzone[1][1][1][7], 1.1f, -0.1f, 0.3f, -1.3f, 0, 1, 0, 0, 0, 0);

	//conti_act
	vFieldContiAct[0][0].set(-0.8f, 0.1f, 0.0f);
	vFieldContiAct[0][1].set(0.0f, 0.1f, 0.0f);
	vFieldContiAct[0][2].set(-0.8f, 1.3f, 0.0f);
	vFieldContiAct[0][3].set(0.0f, 1.3f, 0.0f);
	//conti_act speed
	vFieldContiAct[1][0].set(0.3f, 0.1f, 0.0f);
	vFieldContiAct[1][1].set(1.1f, 0.1f, 0.0f);
	vFieldContiAct[1][2].set(0.3f, 1.3f, 0.0f);
	vFieldContiAct[1][3].set(1.1f, 1.3f, 0.0f);

	//skill card zone
	SetS3DVertex(vSkillZone[0][0][0], 0.0f, 1.4f, -0.8f, 2.6f, 0, 1, 0, 0, 0, 0);
	SetS3DVertex(vSkillZone[1][0][0], 0.2f, 0.1f, 1.0f, 1.3f, 0, 1, 0, 0, 0, 0);
	SetS3DVertex(vSkillZone[0][0][1], 7.9f, -0.1f, 8.7f, -1.3f, 0, 1, 0, 0, 0, 0);
	SetS3DVertex(vSkillZone[1][0][1], 7.7f, -0.1f, 6.9f, -1.3f, 0, 1, 0, 0, 0, 0);

	SetS3DVertex(vSkillZone[0][1][0], 1.1f, 1.4f, 0.3f, 2.6f, 0, 1, 0, 0, 0, 0);
	SetS3DVertex(vSkillZone[1][1][0], 1.1f, 1.4f, 0.3f, 2.6f, 0, 1, 0, 0, 0, 0);
	SetS3DVertex(vSkillZone[0][1][1], 6.8f, -0.1f, 7.6f, -1.3f, 0, 1, 0, 0, 0, 0);
	SetS3DVertex(vSkillZone[1][1][1], 7.7f, -0.1f, 6.9f, -1.3f, 0, 1, 0, 0, 0, 0);


	for(irr::u16 i = 0; i < 40; ++i)
		iArrow[i] = i;

	mCard.AmbientColor = 0xffffffff;
	mCard.DiffuseColor = 0xff000000;
	mCard.ColorMaterial = irr::video::ECM_NONE;
	//mCard.MaterialType = irr::video::EMT_ONETEXTURE_BLEND;
	mCard.MaterialType = irr::video::EMT_TRANSPARENT_ALPHA_CHANNEL;
	mCard.MaterialTypeParam = pack_textureBlendFunc(irr::video::EBF_SRC_ALPHA, irr::video::EBF_ONE_MINUS_SRC_ALPHA, irr::video::EMFN_MODULATE_1X, irr::video::EAS_VERTEX_COLOR);
	ENABLE_ZWRITE(mCard);
	mCenter.AmbientColor = 0xffffffff;
	mCenter.DiffuseColor = 0xff000000;
	mCenter.ColorMaterial = irr::video::ECM_NONE;
	//mCard.MaterialType = irr::video::EMT_ONETEXTURE_BLEND;
	mCenter.MaterialType = irr::video::EMT_TRANSPARENT_ALPHA_CHANNEL;
	mCenter.MaterialTypeParam = pack_textureBlendFunc(irr::video::EBF_SRC_ALPHA, irr::video::EBF_ONE_MINUS_SRC_ALPHA, irr::video::EMFN_MODULATE_1X, irr::video::EAS_VERTEX_COLOR);
	ENABLE_ZWRITE(mCenter);
	mTexture.AmbientColor = 0xffffffff;
	mTexture.DiffuseColor = 0xff000000;
	mTexture.ColorMaterial = irr::video::ECM_NONE;
	mTexture.MaterialType = irr::video::EMT_TRANSPARENT_ALPHA_CHANNEL;
	mBackLine.ColorMaterial = irr::video::ECM_NONE;
	mBackLine.AmbientColor = 0xffffffff;
	mBackLine.DiffuseColor = 0xc0000000;
	mBackLine.AntiAliasing = irr::video::EAAM_FULL_BASIC;
	mBackLine.MaterialType = irr::video::EMT_ONETEXTURE_BLEND;
	mBackLine.MaterialTypeParam = pack_textureBlendFunc(irr::video::EBF_SRC_ALPHA, irr::video::EBF_ONE_MINUS_SRC_ALPHA, irr::video::EMFN_MODULATE_1X, irr::video::EAS_VERTEX_COLOR);
	mBackLine.Thickness = 2;
	ENABLE_ZWRITE(mBackLine);
	mSelField.ColorMaterial = irr::video::ECM_NONE;
	mSelField.AmbientColor = 0xffffffff;
	mSelField.DiffuseColor = 0xff000000;
	mSelField.MaterialType = irr::video::EMT_ONETEXTURE_BLEND;
	mSelField.MaterialTypeParam = pack_textureBlendFunc(irr::video::EBF_SRC_ALPHA, irr::video::EBF_ONE_MINUS_SRC_ALPHA, irr::video::EMFN_MODULATE_1X, irr::video::EAS_VERTEX_COLOR);
	ENABLE_ZWRITE(mSelField);
	mLinkedField.ColorMaterial = irr::video::ECM_NONE;
	mLinkedField.AmbientColor = 0xffffffff;
	mLinkedField.DiffuseColor = 0xff000000;
	mLinkedField.MaterialType = irr::video::EMT_ONETEXTURE_BLEND;
	mLinkedField.MaterialTypeParam = pack_textureBlendFunc(irr::video::EBF_SRC_ALPHA, irr::video::EBF_ONE_MINUS_SRC_ALPHA, irr::video::EMFN_MODULATE_1X, irr::video::EAS_VERTEX_COLOR);
	ENABLE_ZWRITE(mLinkedField);
	mMutualLinkedField.ColorMaterial = irr::video::ECM_NONE;
	mMutualLinkedField.AmbientColor = 0xffffffff;
	mMutualLinkedField.DiffuseColor = 0xff000000;
	mMutualLinkedField.MaterialType = irr::video::EMT_ONETEXTURE_BLEND;
	mMutualLinkedField.MaterialTypeParam = pack_textureBlendFunc(irr::video::EBF_SRC_ALPHA, irr::video::EBF_ONE_MINUS_SRC_ALPHA, irr::video::EMFN_MODULATE_1X, irr::video::EAS_VERTEX_COLOR);
	ENABLE_ZWRITE(mMutualLinkedField);
	mImpInfField.ColorMaterial = irr::video::ECM_NONE;
	mImpInfField.AmbientColor = 0xffffffff;
	mImpInfField.DiffuseColor = 0xff000000;
	mImpInfField.MaterialType = irr::video::EMT_ONETEXTURE_BLEND;
	mImpInfField.MaterialTypeParam = pack_textureBlendFunc(irr::video::EBF_SRC_ALPHA, irr::video::EBF_ONE_MINUS_SRC_ALPHA, irr::video::EMFN_MODULATE_1X, irr::video::EAS_VERTEX_COLOR);
	ENABLE_ZWRITE(mImpInfField);
	mAttFireField.ColorMaterial = irr::video::ECM_NONE;
	mAttFireField.AmbientColor = 0xffffffff;
	mAttFireField.DiffuseColor = 0xff000000;
	mAttFireField.MaterialType = irr::video::EMT_ONETEXTURE_BLEND;
	mAttFireField.MaterialTypeParam = pack_textureBlendFunc(irr::video::EBF_SRC_ALPHA, irr::video::EBF_ONE_MINUS_SRC_ALPHA, irr::video::EMFN_MODULATE_1X, irr::video::EAS_VERTEX_COLOR);
	ENABLE_ZWRITE(mAttFireField);
	mAttEarthField.ColorMaterial = irr::video::ECM_NONE;
	mAttEarthField.AmbientColor = 0xffffffff;
	mAttEarthField.DiffuseColor = 0xff000000;
	mAttEarthField.MaterialType = irr::video::EMT_ONETEXTURE_BLEND;
	mAttEarthField.MaterialTypeParam = pack_textureBlendFunc(irr::video::EBF_SRC_ALPHA, irr::video::EBF_ONE_MINUS_SRC_ALPHA, irr::video::EMFN_MODULATE_1X, irr::video::EAS_VERTEX_COLOR);
	ENABLE_ZWRITE(mAttEarthField);
	mAttLightField.ColorMaterial = irr::video::ECM_NONE;
	mAttLightField.AmbientColor = 0xffffffff;
	mAttLightField.DiffuseColor = 0xff000000;
	mAttLightField.MaterialType = irr::video::EMT_ONETEXTURE_BLEND;
	mAttLightField.MaterialTypeParam = pack_textureBlendFunc(irr::video::EBF_SRC_ALPHA, irr::video::EBF_ONE_MINUS_SRC_ALPHA, irr::video::EMFN_MODULATE_1X, irr::video::EAS_VERTEX_COLOR);
	ENABLE_ZWRITE(mAttLightField);
	mAttWindField.ColorMaterial = irr::video::ECM_NONE;
	mAttWindField.AmbientColor = 0xffffffff;
	mAttWindField.DiffuseColor = 0xff000000;
	mAttWindField.MaterialType = irr::video::EMT_ONETEXTURE_BLEND;
	mAttWindField.MaterialTypeParam = pack_textureBlendFunc(irr::video::EBF_SRC_ALPHA, irr::video::EBF_ONE_MINUS_SRC_ALPHA, irr::video::EMFN_MODULATE_1X, irr::video::EAS_VERTEX_COLOR);
	ENABLE_ZWRITE(mAttWindField);
	mAttWaterField.ColorMaterial = irr::video::ECM_NONE;
	mAttWaterField.AmbientColor = 0xffffffff;
	mAttWaterField.DiffuseColor = 0xff000000;
	mAttWaterField.MaterialType = irr::video::EMT_ONETEXTURE_BLEND;
	mAttWaterField.MaterialTypeParam = pack_textureBlendFunc(irr::video::EBF_SRC_ALPHA, irr::video::EBF_ONE_MINUS_SRC_ALPHA, irr::video::EMFN_MODULATE_1X, irr::video::EAS_VERTEX_COLOR);
	ENABLE_ZWRITE(mAttWaterField);
	mAttDarkField.ColorMaterial = irr::video::ECM_NONE;
	mAttDarkField.AmbientColor = 0xffffffff;
	mAttDarkField.DiffuseColor = 0xff000000;
	mAttDarkField.MaterialType = irr::video::EMT_ONETEXTURE_BLEND;
	mAttDarkField.MaterialTypeParam = pack_textureBlendFunc(irr::video::EBF_SRC_ALPHA, irr::video::EBF_ONE_MINUS_SRC_ALPHA, irr::video::EMFN_MODULATE_1X, irr::video::EAS_VERTEX_COLOR);
	ENABLE_ZWRITE(mAttDarkField);
	mAttDivineField.ColorMaterial = irr::video::ECM_NONE;
	mAttDivineField.AmbientColor = 0xffffffff;
	mAttDivineField.DiffuseColor = 0xff000000;
	mAttDivineField.MaterialType = irr::video::EMT_ONETEXTURE_BLEND;
	mAttDivineField.MaterialTypeParam = pack_textureBlendFunc(irr::video::EBF_SRC_ALPHA, irr::video::EBF_ONE_MINUS_SRC_ALPHA, irr::video::EMFN_MODULATE_1X, irr::video::EAS_VERTEX_COLOR);
	ENABLE_ZWRITE(mAttDivineField);
	mTypeSpellField.ColorMaterial = irr::video::ECM_NONE;
	mTypeSpellField.AmbientColor = 0xffffffff;
	mTypeSpellField.DiffuseColor = 0xff000000;
	mTypeSpellField.MaterialType = irr::video::EMT_ONETEXTURE_BLEND;
	mTypeSpellField.MaterialTypeParam = pack_textureBlendFunc(irr::video::EBF_SRC_ALPHA, irr::video::EBF_ONE_MINUS_SRC_ALPHA, irr::video::EMFN_MODULATE_1X, irr::video::EAS_VERTEX_COLOR);
	ENABLE_ZWRITE(mTypeSpellField);
	mTypeTrapField.ColorMaterial = irr::video::ECM_NONE;
	mTypeTrapField.AmbientColor = 0xffffffff;
	mTypeTrapField.DiffuseColor = 0xff000000;
	mTypeTrapField.MaterialType = irr::video::EMT_ONETEXTURE_BLEND;
	mTypeTrapField.MaterialTypeParam = pack_textureBlendFunc(irr::video::EBF_SRC_ALPHA, irr::video::EBF_ONE_MINUS_SRC_ALPHA, irr::video::EMFN_MODULATE_1X, irr::video::EAS_VERTEX_COLOR);
	ENABLE_ZWRITE(mTypeTrapField);
	mSquareBlackField.ColorMaterial = irr::video::ECM_NONE;
	mSquareBlackField.AmbientColor = 0xffffffff;
	mSquareBlackField.DiffuseColor = 0xff000000;
	mSquareBlackField.MaterialType = irr::video::EMT_ONETEXTURE_BLEND;
	mSquareBlackField.MaterialTypeParam = pack_textureBlendFunc(irr::video::EBF_SRC_ALPHA, irr::video::EBF_ONE_MINUS_SRC_ALPHA, irr::video::EMFN_MODULATE_1X, irr::video::EAS_VERTEX_COLOR);
	ENABLE_ZWRITE(mSquareBlackField);
	mOutLine.ColorMaterial = irr::video::ECM_AMBIENT;
	mOutLine.DiffuseColor = 0xff000000;
	mOutLine.Thickness = 2;
	mTRTexture = mTexture;
	mTRTexture.AmbientColor = 0xffffff00;
	mATK.ColorMaterial = irr::video::ECM_NONE;
	mATK.setFlag(irr::video::EMF_BACK_FACE_CULLING, 0);
	mATK.MaterialType = irr::video::EMT_ONETEXTURE_BLEND;
	mATK.MaterialTypeParam = pack_textureBlendFunc(irr::video::EBF_SRC_ALPHA, irr::video::EBF_ONE_MINUS_SRC_ALPHA, irr::video::EMFN_MODULATE_1X, irr::video::EAS_VERTEX_COLOR);
	ENABLE_ZWRITE(mATK);
}
void Materials::GenArrow(float y) {
	float ay = 1.0f;
	for (int i = 0; i < 19; ++i) {
		vArrow[i * 2] = vArrow[i * 2 + 1] = irr::video::S3DVertex(irr::core::vector3df(0.1f, ay * y, -2.0f * (ay * ay - 1.0f)), irr::core::vector3df(0, ay * y, 1), skin::DUELFIELD_ATTACK_ARROW_VAL, irr::core::vector2df(0, 0));
		vArrow[i * 2 + 1].Pos.X = -0.1f;
		ay -= 0.1f;
	}
	vArrow[36].Pos.X = 0.2f;
	vArrow[36].Pos.Y = vArrow[34].Pos.Y - 0.01f;
	vArrow[36].Pos.Z = vArrow[34].Pos.Z - 0.01f;
	vArrow[37].Pos.X = -0.2f;
	vArrow[37].Pos.Y = vArrow[35].Pos.Y - 0.01f;
	vArrow[37].Pos.Z = vArrow[35].Pos.Z - 0.01f;
	vArrow[38] = irr::video::S3DVertex(irr::core::vector3df(0.0f, -1.0f * y, 0.0f), irr::core::vector3df(0.0f, -1.0f, -1.0f), 0xc0ffffff, irr::core::vector2df(0, 0));
	vArrow[39] = vArrow[38];
}

void Materials::SetActiveVertices(int three_columns, int not_separate_pzones) {
	vActiveSzone = &vFieldSzone[not_separate_pzones][three_columns];
	vActiveDeck = &vFieldDeck[three_columns];
	vActiveExtra = &vFieldExtra[three_columns];
	vActiveGrave = &vFieldGrave[not_separate_pzones][three_columns];
	vActiveRemove = &vFieldRemove[not_separate_pzones][three_columns];
	vActiveSkill = &vSkillZone[not_separate_pzones][three_columns];
}

}
