// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
#pragma once
#include <cstdint>
#include <iostream>
#include "Types.h"

namespace CGame
{ 
	struct CGame
	{
		/*
		const SGameLevels* gameLevel;
		CGame::textMessage textMessageList[6];
		unsigned int fadeGoalARGB;
		unsigned int fadeARGB;
		float fadeTimeRemaining;
		CMaterial* possessedGlowMtl;
		float mDifficultyScalar;
		int mHeroCountAtMatchStart;
		EDifficulty difficulty;
		bool creditsQueued;
		float reticleOpacity;
		int showHUDDamageTallyFlag;
		int showHUDRewardTallyFlag;
		EGameControl gameControl;
		SControls controlsList[4];
		int forceQuit;
		CGame::SLight* activeLightList[8];
		CGame::SLight lightList[8];
		char debugString[256];
		bool screenShotMode;
		float clampedFPS;
		bool clampFPS;
		bool gamePaused;
		float totalGameplayTime;
		float levelTime;
		int firstFrameBlack;
		bool warpScreen;
		float warpFactor;
		unsigned int debugMode;
		bool mIsSaving;
		int sizeMode;
		bool displayClueMode;
		bool pausedTutorialMode;
		CGame::EAutoPageType menuAutoPageType;
		float menuAutoPageTimer;
		HSoundInstance hDbNarrativeSoundInstance;
		bool allowDamageTallyFlag;
		bool chainingLevels;
		CGame::SCheckpoint checkpoints[25];
		float saveWarningMessageTimer;
		CStatSystem statSystem;
		CAchievementSystem achievementSystem;
		CHeatMap heatMap;
		float blockAITalkingTimer;
		int pkeSourceCount;
		CActor* pkeSourceList[256];
		float soundMasterVolume;
		float soundEffectsVolume;
		float soundDialogVolume;
		float soundMusicVolume;
		float soundAmbienceVolume;
		float soundFMVVolume;
		*/
	};

	// this + 0x3ADC0: the head of the engine's intrusive actor chain, every actor in the level, the spawn pool included.
	static constexpr uintptr_t actorListHead = 0x3ADC0;

	void enableDebug(bool enabled);
	int slew();
	void dbNarrative(CDialogDatabaseEntry dbEntry);
	void chainToLevel(const char* levelName, const char* checkpointName);
}