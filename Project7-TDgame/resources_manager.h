#pragma once
#include"manager.h"
#include<SDL.h>
#include<SDL_ttf.h>
#include<SDL_mixer.h>
#include<SDL_image.h>
#include<unordered_map>



enum class ResID
{
	Tex_Tileset,

	Tex_Player,
	Tex_Archer,
	Tex_Axeman,
	Tex_Gunner,
	Tex_BarracksLv1,
	Tex_BarracksLv2,
	Tex_BarracksLv3,
	Tex_BarracksShieldWall,
	Tex_BarracksVanguard,
	Tex_BarracksSoldierIdle,
	Tex_BarracksSoldierWalk,
	Tex_BarracksSoldierAttack,
	Tex_BarracksSoldierHit,

	Tex_Slime,
	Tex_KingSlime,
	Tex_Skeleton,
	Tex_Goblin,
	Tex_GoblinPriest,
	Tex_Boss,
	Tex_SilencerIdle,
	Tex_SilencerWalk,
	Tex_SilencerCast,
	Tex_ArmoredIdle,
	Tex_ArmoredWalk,
	Tex_ArmoredHit,
	Tex_SlimeSketch,
	Tex_KingSlimeSketch,
	Tex_SkeletonSketch,
	Tex_GoblinSketch,
	Tex_GoblinPriestSketch,
	Tex_BossSketch,

	Tex_BulletArrow,
	Tex_BulletAxe,
	Tex_BulletShell,

	Tex_Coin,
	Tex_Home,

	Tex_EffectFlash_Up,
	Tex_EffectFlash_Down,
	Tex_EffectFlash_Left,
	Tex_EffectFlash_Right,
	Tex_EffectImpact_Up,
	Tex_EffectImpact_Down,
	Tex_EffectImpact_Left,
	Tex_EffectImpact_Right,
	Tex_EffectExplode,
	Tex_EffectSilenceArea,
	Tex_EffectSilenceProjectile,
	Tex_EffectArmorBreakHit,
	Tex_EffectSlowHit,
	Tex_EffectSoldierSlash,

	Tex_UISelectCursor,
	Tex_UIPlaceIdle,
	Tex_UIPlaceHoveredTop,
	Tex_UIPlaceHoveredLeft,
	Tex_UIPlaceHoveredRight,
	Tex_UIUpgradeIdle,
	Tex_UIUpgradeHoveredTop,
	Tex_UIUpgradeHoveredLeft,
	Tex_UIUpgradeHoveredRight,
	Tex_UIHomeAvatar,
	Tex_UIPlayerAvatar,
	Tex_UIHeart,
	Tex_UICoin,
	Tex_UIGameOverBar,
	Tex_UIWinText,
	Tex_UILossText,
	Tex_UIButtonIdle,
	Tex_UIButtonHover,
	Tex_UIButtonDisabled,
	Tex_UILevelCardBg,
	Tex_UILevelCardHover,
	Tex_UILevelLockedMask,
	Tex_UILockIcon,
	Tex_UIResultPanelBg,
	Tex_UIResultVictory,
	Tex_UIResultDefeat,
	Tex_UIStarFull,
	Tex_UIStarEmpty,
	Tex_UICommandPanelBg,
	Tex_UIBuildPanel,
	Tex_UIBuildPanelHover0,
	Tex_UIBuildPanelHover1,
	Tex_UIBuildPanelHover2,
	Tex_UIBuildPanelHover3,
	Tex_UITowerUpgradePanel,
	Tex_UITowerUpgradePanelHoverUpgrade,
	Tex_UITowerUpgradePanelHoverSpecLeft,
	Tex_UITowerUpgradePanelHoverSpecRight,
	Tex_UIRemoveTowerPanel,
	Tex_UIRemoveTowerPanelHoverConfirm,
	Tex_UIRemoveTowerPanelHoverCancel,
	Tex_UIStatusSilence,
	Tex_UIStatusArmor,
	Tex_UIStatusIntercept,
	Tex_UISpecArcherRapidFire,
	Tex_UISpecArcherPiercingArmor,
	Tex_UISpecAxemanStrongSlow,
	Tex_UISpecAxemanCleave,
	Tex_UISpecGunnerBarrage,
	Tex_UISpecGunnerShotgun,
	Tex_UISpecBarracksShieldWall,
	Tex_UISpecBarracksVanguard,
	Tex_UIBarracksShowcase,
	Tex_UIEnemySpawnAvatarSilencer,
	Tex_UIEnemySpawnAvatarArmored,

	Sound_ArrowFire_1,
	Sound_ArrowFire_2,
	Sound_AxeFire,
	Sound_ShellFire,
	Sound_ArrowHit_1,
	Sound_ArrowHit_2,
	Sound_ArrowHit_3,
	Sound_AxeHit_1,
	Sound_AxeHit_2,
	Sound_AxeHit_3,
	Sound_ShellHit,

	Sound_Flash,
	Sound_Impact,

	Sound_Coin,
	Sound_HomeHurt,
	Sound_PlaceTower,
	Sound_TowerLevelUp,

	Sound_Win,
	Sound_Loss,

	Music_BGM,

	Font_Main,
	Font_Small
};

class ResourcesManager : public Manager<ResourcesManager>
{
	friend class Manager<ResourcesManager>;
protected:
	ResourcesManager() = default;
	~ResourcesManager() = default;
public:
	typedef std::unordered_map<ResID, TTF_Font*> FontPool;
	typedef std::unordered_map<ResID, Mix_Chunk*> SoundPool;
	typedef std::unordered_map<ResID, Mix_Music*> MusicPool;
	typedef std::unordered_map<ResID, SDL_Texture*> TexturePool;

public:
	bool load_from_file(SDL_Renderer* renderer)
	{
		texture_pool[ResID::Tex_Tileset] = IMG_LoadTexture(renderer, "resources/tileset.png");


		texture_pool[ResID::Tex_Player] = IMG_LoadTexture(renderer, "resources/player.png");
		texture_pool[ResID::Tex_Archer] = IMG_LoadTexture(renderer, "resources/tower_archer.png");
		texture_pool[ResID::Tex_Axeman] = IMG_LoadTexture(renderer, "resources/tower_axeman.png");
		texture_pool[ResID::Tex_Gunner] = IMG_LoadTexture(renderer, "resources/tower_gunner.png");
		texture_pool[ResID::Tex_BarracksLv1] = IMG_LoadTexture(renderer, "resources/barracks_tower_lv1.png");
		texture_pool[ResID::Tex_BarracksLv2] = IMG_LoadTexture(renderer, "resources/barracks_tower_lv2.png");
		texture_pool[ResID::Tex_BarracksLv3] = IMG_LoadTexture(renderer, "resources/barracks_tower_lv3.png");
		texture_pool[ResID::Tex_BarracksShieldWall] = IMG_LoadTexture(renderer, "resources/barracks_tower_shield_wall.png");
		texture_pool[ResID::Tex_BarracksVanguard] = IMG_LoadTexture(renderer, "resources/barracks_tower_vanguard.png");
		texture_pool[ResID::Tex_BarracksSoldierIdle] = IMG_LoadTexture(renderer, "resources/barracks_soldier_idle.png");
		texture_pool[ResID::Tex_BarracksSoldierWalk] = IMG_LoadTexture(renderer, "resources/barracks_soldier_walk.png");
		texture_pool[ResID::Tex_BarracksSoldierAttack] = IMG_LoadTexture(renderer, "resources/barracks_soldier_attack.png");
		texture_pool[ResID::Tex_BarracksSoldierHit] = IMG_LoadTexture(renderer, "resources/barracks_soldier_attack.png");

		texture_pool[ResID::Tex_Slime] = IMG_LoadTexture(renderer, "resources/enemy_slime.png");
		texture_pool[ResID::Tex_KingSlime] = IMG_LoadTexture(renderer, "resources/enemy_king_slime.png");
		texture_pool[ResID::Tex_Skeleton] = IMG_LoadTexture(renderer, "resources/enemy_skeleton.png");
		texture_pool[ResID::Tex_Goblin] = IMG_LoadTexture(renderer, "resources/enemy_goblin.png");
		texture_pool[ResID::Tex_GoblinPriest] = IMG_LoadTexture(renderer, "resources/enemy_goblin_priest.png");
		texture_pool[ResID::Tex_Boss] = IMG_LoadTexture(renderer, "resources/enemy_boss.png");
		texture_pool[ResID::Tex_SilencerIdle] = IMG_LoadTexture(renderer, "resources/enemy_silencer_walk.png");
		texture_pool[ResID::Tex_SilencerWalk] = IMG_LoadTexture(renderer, "resources/enemy_silencer_walk.png");
		texture_pool[ResID::Tex_SilencerCast] = IMG_LoadTexture(renderer, "resources/enemy_silencer_cast.png");
		texture_pool[ResID::Tex_ArmoredIdle] = IMG_LoadTexture(renderer, "resources/enemy_armored_walk.png");
		texture_pool[ResID::Tex_ArmoredWalk] = IMG_LoadTexture(renderer, "resources/enemy_armored_walk.png");
		texture_pool[ResID::Tex_ArmoredHit] = IMG_LoadTexture(renderer, "resources/enemy_armored_hit.png");
		texture_pool[ResID::Tex_SlimeSketch] = IMG_LoadTexture(renderer, "resources/enemy_slime_sketch.png");
		texture_pool[ResID::Tex_KingSlimeSketch] = IMG_LoadTexture(renderer, "resources/enemy_king_slime_sketch.png");
		texture_pool[ResID::Tex_SkeletonSketch] = IMG_LoadTexture(renderer, "resources/enemy_skeleton_sketch.png");
		texture_pool[ResID::Tex_GoblinSketch] = IMG_LoadTexture(renderer, "resources/enemy_goblin_sketch.png");
		texture_pool[ResID::Tex_GoblinPriestSketch] = IMG_LoadTexture(renderer, "resources/enemy_goblin_priest_sketch.png");
		texture_pool[ResID::Tex_BossSketch] = IMG_LoadTexture(renderer, "resources/enemy_boss_sketch.png");

		texture_pool[ResID::Tex_BulletArrow] = IMG_LoadTexture(renderer, "resources/bullet_arrow.png");
		texture_pool[ResID::Tex_BulletAxe] = IMG_LoadTexture(renderer, "resources/bullet_axe.png");
		texture_pool[ResID::Tex_BulletShell] = IMG_LoadTexture(renderer, "resources/bullet_shell.png");

		texture_pool[ResID::Tex_Coin] = IMG_LoadTexture(renderer, "resources/coin.png");
		texture_pool[ResID::Tex_Home] = IMG_LoadTexture(renderer, "resources/home.png");

		texture_pool[ResID::Tex_EffectFlash_Up] = IMG_LoadTexture(renderer, "resources/effect_flash_up.png");
		texture_pool[ResID::Tex_EffectFlash_Down] = IMG_LoadTexture(renderer, "resources/effect_flash_down.png");
		texture_pool[ResID::Tex_EffectFlash_Left] = IMG_LoadTexture(renderer, "resources/effect_flash_left.png");
		texture_pool[ResID::Tex_EffectFlash_Right] = IMG_LoadTexture(renderer, "resources/effect_flash_right.png");
		texture_pool[ResID::Tex_EffectImpact_Up] = IMG_LoadTexture(renderer, "resources/effect_impact_up.png");
		texture_pool[ResID::Tex_EffectImpact_Down] = IMG_LoadTexture(renderer, "resources/effect_impact_down.png");
		texture_pool[ResID::Tex_EffectImpact_Left] = IMG_LoadTexture(renderer, "resources/effect_impact_left.png");
		texture_pool[ResID::Tex_EffectImpact_Right] = IMG_LoadTexture(renderer, "resources/effect_impact_right.png");
		texture_pool[ResID::Tex_EffectExplode] = IMG_LoadTexture(renderer, "resources/effect_explode.png");
		texture_pool[ResID::Tex_EffectSilenceArea] = IMG_LoadTexture(renderer, "resources/effect_silence_area.png");
		texture_pool[ResID::Tex_EffectSilenceProjectile] = IMG_LoadTexture(renderer, "resources/effect_silence_projectile.png");
		texture_pool[ResID::Tex_EffectArmorBreakHit] = IMG_LoadTexture(renderer, "resources/effect_armor_break_hit.png");
		texture_pool[ResID::Tex_EffectSlowHit] = IMG_LoadTexture(renderer, "resources/effect_slow_hit.png");
		texture_pool[ResID::Tex_EffectSoldierSlash] = IMG_LoadTexture(renderer, "resources/effect_soldier_slash.png");

		texture_pool[ResID::Tex_UISelectCursor] = IMG_LoadTexture(renderer, "resources/ui_select_cursor.png");
		texture_pool[ResID::Tex_UIPlaceIdle] = IMG_LoadTexture(renderer, "resources/ui_place_idle.png");
		texture_pool[ResID::Tex_UIPlaceHoveredTop] = IMG_LoadTexture(renderer, "resources/ui_place_hovered_top.png");
		texture_pool[ResID::Tex_UIPlaceHoveredLeft] = IMG_LoadTexture(renderer, "resources/ui_place_hovered_left.png");
		texture_pool[ResID::Tex_UIPlaceHoveredRight] = IMG_LoadTexture(renderer, "resources/ui_place_hovered_right.png");
		texture_pool[ResID::Tex_UIUpgradeIdle] = IMG_LoadTexture(renderer, "resources/ui_upgrade_idle.png");
		texture_pool[ResID::Tex_UIUpgradeHoveredTop] = IMG_LoadTexture(renderer, "resources/ui_upgrade_hovered_top.png");
		texture_pool[ResID::Tex_UIUpgradeHoveredLeft] = IMG_LoadTexture(renderer, "resources/ui_upgrade_hovered_left.png");
		texture_pool[ResID::Tex_UIUpgradeHoveredRight] = IMG_LoadTexture(renderer, "resources/ui_upgrade_hovered_right.png");
		texture_pool[ResID::Tex_UIHomeAvatar] = IMG_LoadTexture(renderer, "resources/ui_home_avatar.png");
		texture_pool[ResID::Tex_UIPlayerAvatar] = IMG_LoadTexture(renderer, "resources/ui_player_avatar.png");
		texture_pool[ResID::Tex_UIHeart] = IMG_LoadTexture(renderer, "resources/ui_heart.png");
		texture_pool[ResID::Tex_UICoin] = IMG_LoadTexture(renderer, "resources/ui_coin.png");
		texture_pool[ResID::Tex_UIGameOverBar] = IMG_LoadTexture(renderer, "resources/ui_game_over_bar.png");
		texture_pool[ResID::Tex_UIWinText] = IMG_LoadTexture(renderer, "resources/ui_win_text.png");
		texture_pool[ResID::Tex_UILossText] = IMG_LoadTexture(renderer, "resources/ui_loss_text.png");
		texture_pool[ResID::Tex_UIButtonIdle] = IMG_LoadTexture(renderer, "resources/ui_button_idle.png");
		texture_pool[ResID::Tex_UIButtonHover] = IMG_LoadTexture(renderer, "resources/ui_button_hover.png");
		texture_pool[ResID::Tex_UIButtonDisabled] = IMG_LoadTexture(renderer, "resources/ui_button_disabled.png");
		texture_pool[ResID::Tex_UILevelCardBg] = IMG_LoadTexture(renderer, "resources/ui_level_card_bg.png");
		texture_pool[ResID::Tex_UILevelCardHover] = IMG_LoadTexture(renderer, "resources/ui_level_card_hover.png");
		texture_pool[ResID::Tex_UILevelLockedMask] = IMG_LoadTexture(renderer, "resources/ui_level_locked_mask.png");
		texture_pool[ResID::Tex_UILockIcon] = IMG_LoadTexture(renderer, "resources/ui_lock_icon.png");
		texture_pool[ResID::Tex_UIResultPanelBg] = IMG_LoadTexture(renderer, "resources/ui_result_panel_bg.png");
		texture_pool[ResID::Tex_UIResultVictory] = IMG_LoadTexture(renderer, "resources/ui_result_victory.png");
		texture_pool[ResID::Tex_UIResultDefeat] = IMG_LoadTexture(renderer, "resources/ui_result_defeat.png");
		texture_pool[ResID::Tex_UIStarFull] = IMG_LoadTexture(renderer, "resources/ui_star_full.png");
		texture_pool[ResID::Tex_UIStarEmpty] = IMG_LoadTexture(renderer, "resources/ui_star_empty.png");
		texture_pool[ResID::Tex_UICommandPanelBg] = IMG_LoadTexture(renderer, "resources/ui_command_panel_bg.png");
		texture_pool[ResID::Tex_UIBuildPanel] = IMG_LoadTexture(renderer, "resources/ui_build_panel.png");
		texture_pool[ResID::Tex_UIBuildPanelHover0] = IMG_LoadTexture(renderer, "resources/ui_build_panel_hover_0.png");
		texture_pool[ResID::Tex_UIBuildPanelHover1] = IMG_LoadTexture(renderer, "resources/ui_build_panel_hover_1.png");
		texture_pool[ResID::Tex_UIBuildPanelHover2] = IMG_LoadTexture(renderer, "resources/ui_build_panel_hover_2.png");
		texture_pool[ResID::Tex_UIBuildPanelHover3] = IMG_LoadTexture(renderer, "resources/ui_build_panel_hover_3.png");
		texture_pool[ResID::Tex_UITowerUpgradePanel] = IMG_LoadTexture(renderer, "resources/ui_tower_upgrade_panel.png");
		texture_pool[ResID::Tex_UITowerUpgradePanelHoverUpgrade] = IMG_LoadTexture(renderer, "resources/ui_tower_upgrade_panel_hover_upgrade.png");
		texture_pool[ResID::Tex_UITowerUpgradePanelHoverSpecLeft] = IMG_LoadTexture(renderer, "resources/ui_tower_upgrade_panel_hover_spec_left.png");
		texture_pool[ResID::Tex_UITowerUpgradePanelHoverSpecRight] = IMG_LoadTexture(renderer, "resources/ui_tower_upgrade_panel_hover_spec_right.png");
		texture_pool[ResID::Tex_UIRemoveTowerPanel] = IMG_LoadTexture(renderer, "resources/ui_remove_tower_panel.png");
		texture_pool[ResID::Tex_UIRemoveTowerPanelHoverConfirm] = IMG_LoadTexture(renderer, "resources/ui_remove_tower_panel_hover_confirm.png");
		texture_pool[ResID::Tex_UIRemoveTowerPanelHoverCancel] = IMG_LoadTexture(renderer, "resources/ui_remove_tower_panel_hover_cancel.png");
		texture_pool[ResID::Tex_UIStatusSilence] = IMG_LoadTexture(renderer, "resources/ui_status_silence.png");
		texture_pool[ResID::Tex_UIStatusArmor] = IMG_LoadTexture(renderer, "resources/ui_status_armor.png");
		texture_pool[ResID::Tex_UIStatusIntercept] = IMG_LoadTexture(renderer, "resources/ui_status_intercept.png");
		texture_pool[ResID::Tex_UISpecArcherRapidFire] = IMG_LoadTexture(renderer, "resources/ui_spec_archer_rapid_fire.png");
		texture_pool[ResID::Tex_UISpecArcherPiercingArmor] = IMG_LoadTexture(renderer, "resources/ui_spec_archer_piercing_armor.png");
		texture_pool[ResID::Tex_UISpecAxemanStrongSlow] = IMG_LoadTexture(renderer, "resources/ui_spec_axeman_strong_slow.png");
		texture_pool[ResID::Tex_UISpecAxemanCleave] = IMG_LoadTexture(renderer, "resources/ui_spec_axeman_cleave.png");
		texture_pool[ResID::Tex_UISpecGunnerBarrage] = IMG_LoadTexture(renderer, "resources/ui_spec_gunner_barrage.png");
		texture_pool[ResID::Tex_UISpecGunnerShotgun] = IMG_LoadTexture(renderer, "resources/ui_spec_gunner_shotgun.png");
		texture_pool[ResID::Tex_UISpecBarracksShieldWall] = IMG_LoadTexture(renderer, "resources/ui_spec_barracks_shield_wall.png");
		texture_pool[ResID::Tex_UISpecBarracksVanguard] = IMG_LoadTexture(renderer, "resources/ui_spec_barracks_vanguard.png");
		texture_pool[ResID::Tex_UIBarracksShowcase] = IMG_LoadTexture(renderer, "resources/ui_barracks_showcase.png");
		texture_pool[ResID::Tex_UIEnemySpawnAvatarSilencer] = IMG_LoadTexture(renderer, "resources/ui_enemy_spawn_avatar_silencer.png");
		texture_pool[ResID::Tex_UIEnemySpawnAvatarArmored] = IMG_LoadTexture(renderer, "resources/ui_enemy_spawn_avatar_armored.png");
        
		for (const auto& pair : texture_pool)
			if (!pair.second) return false;

		sound_pool[ResID::Sound_ArrowFire_1] = Mix_LoadWAV("resources/sound_arrow_fire_1.mp3");
		sound_pool[ResID::Sound_ArrowFire_2] = Mix_LoadWAV("resources/sound_arrow_fire_2.mp3");
		sound_pool[ResID::Sound_AxeFire] = Mix_LoadWAV("resources/sound_axe_fire.wav");
		sound_pool[ResID::Sound_ShellFire] = Mix_LoadWAV("resources/sound_shell_fire.wav");
		sound_pool[ResID::Sound_ArrowHit_1] = Mix_LoadWAV("resources/sound_arrow_hit_1.mp3");
		sound_pool[ResID::Sound_ArrowHit_2] = Mix_LoadWAV("resources/sound_arrow_hit_2.mp3");
		sound_pool[ResID::Sound_ArrowHit_3] = Mix_LoadWAV("resources/sound_arrow_hit_3.mp3");
		sound_pool[ResID::Sound_AxeHit_1] = Mix_LoadWAV("resources/sound_axe_hit_1.mp3");
		sound_pool[ResID::Sound_AxeHit_2] = Mix_LoadWAV("resources/sound_axe_hit_2.mp3");
		sound_pool[ResID::Sound_AxeHit_3] = Mix_LoadWAV("resources/sound_axe_hit_3.mp3");
		sound_pool[ResID::Sound_ShellHit] = Mix_LoadWAV("resources/sound_shell_hit.mp3");

		sound_pool[ResID::Sound_Flash] = Mix_LoadWAV("resources/sound_flash.wav");
		sound_pool[ResID::Sound_Impact] = Mix_LoadWAV("resources/sound_impact.wav");

		sound_pool[ResID::Sound_Coin] = Mix_LoadWAV("resources/sound_coin.mp3");
		sound_pool[ResID::Sound_HomeHurt] = Mix_LoadWAV("resources/sound_home_hurt.wav");
		sound_pool[ResID::Sound_PlaceTower] = Mix_LoadWAV("resources/sound_place_tower.mp3");
		sound_pool[ResID::Sound_TowerLevelUp] = Mix_LoadWAV("resources/sound_tower_level_up.mp3");

		sound_pool[ResID::Sound_Win] = Mix_LoadWAV("resources/sound_win.wav");
		sound_pool[ResID::Sound_Loss] = Mix_LoadWAV("resources/sound_loss.mp3");

		for (const auto& pair : sound_pool)
			if (!pair.second) return false;

		music_pool[ResID::Music_BGM] = Mix_LoadMUS("resources/music_bgm.mp3");

		for (const auto& pair : music_pool)
			if (!pair.second) return false;

		font_pool[ResID::Font_Main] = TTF_OpenFont("resources/ipix.ttf", 25);
		font_pool[ResID::Font_Small] = TTF_OpenFont("resources/ipix.ttf", 18);

		for (const auto& pair : font_pool)
			if (!pair.second) return false;

		return true;
	}

public:
	const FontPool& get_font_pool() const
	{
		return font_pool;
	}
	  
	const SoundPool& get_sound_pool() const
	{
		return sound_pool;
	}

	const MusicPool& get_music_pool() const
	{
		return music_pool;
	}

	const TexturePool& get_texture_pool() const
	{
		return texture_pool;
	}

private:
	FontPool font_pool;
	SoundPool sound_pool;
	MusicPool music_pool;
	TexturePool texture_pool;

};


