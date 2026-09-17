// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
#pragma once
// Script-native and object addresses in ghost.exe 0b89556c07e5b737efe444351227e747, ghost-relative. Most are

#include <cstdint>

namespace Offsets
{
    static constexpr uintptr_t gGameBase = 0x23229C0;
    static constexpr uintptr_t gGame = 0xDCF680;
    static constexpr uintptr_t getOrient = 0x2BDB50;
    static constexpr uintptr_t enableActorsInsideMe = 0x4A67F0;
    static constexpr uintptr_t findActorByName = 0x2DB760;
    static constexpr uintptr_t exportGlobalVariable = 0x2CED00;
    static constexpr uintptr_t startPreparedTalking = 0x75A00;
    static constexpr uintptr_t setGhostbusterHeatlhState = 0xD0280;
    static constexpr uintptr_t flinch = 0xD18B0;
    static constexpr uintptr_t Singleton_getRoom = 0x2BDBD0;
    static constexpr uintptr_t setMusic = 0x411FE0;
    static constexpr uintptr_t hideHack = 0xEE2D0;
    static constexpr uintptr_t removeSlimeDecals = 0x30D620;
    static constexpr uintptr_t dbNarrativeStop = 0x1F8410;
    static constexpr uintptr_t dbNarrative = 0x1ECEB0;
    static constexpr uintptr_t findDBEntry = 0x2CF5E0;
    static constexpr uintptr_t DanteVMInstance = 0x222D41C;
    static constexpr uintptr_t Singleton_newActor = 0x2C0D50;
    static constexpr uintptr_t die = 0x3B27B0;
    static constexpr uintptr_t blockHeroMovement = 0xED660;
    static constexpr uintptr_t toggleHuntMode = 0xED480;
    static constexpr uintptr_t enableProtonTorpedo = 0xEDE30;
    static constexpr uintptr_t setCommandCrossBeam = 0xEC640;
    static constexpr uintptr_t startFakePackOverheat = 0xED750;
    static constexpr uintptr_t letterbox = 0x2D87A0;
    static constexpr uintptr_t queueVideo = 0x2D87E0;
    static constexpr uintptr_t setMovieCaptureEnable = 0x2D8850;
    static constexpr uintptr_t allowEnemyAttack = 0x2D8420;
    static constexpr uintptr_t allowHeroControls = 0x2D8440;
    static constexpr uintptr_t allowHeroDamage = 0x2D8460;
    static constexpr uintptr_t play = 0x1C3F0;
    static constexpr uintptr_t setCameraPathActor = 0x1FF760;
    static constexpr uintptr_t shatter = 0x49EC70;
    static constexpr uintptr_t setSimEnable = 0x89940;
    static constexpr uintptr_t loadCheckpoint = 0x1F81F0;
    static constexpr uintptr_t setCurrentObjective = 0x1F8200;
    static constexpr uintptr_t toggleReviveMode = 0xD0EE0;
    static constexpr uintptr_t chainToLevel = 0x1EF700;
    static constexpr uintptr_t transferHeroshipTo = 0xD81A0;
    static constexpr uintptr_t slimeMe = 0xD0F50;
    static constexpr uintptr_t knockBack = 0xED100;
    static constexpr uintptr_t pretendToDrive = 0xEAC40;
    static constexpr uintptr_t mountProtonPack = 0xE4640;
    static constexpr uintptr_t fakeFireProtonGun = 0xE9060;
    static constexpr uintptr_t cacheRappel = 0xE1D70;
    static constexpr uintptr_t forceDeployTrap = 0xE4C80;
    static constexpr uintptr_t setRappelModeEnable = 0xE1F70;
    static constexpr uintptr_t startRappelSwing = 0xE21E0;
    static constexpr uintptr_t isDead = 0x7B170;
    static constexpr uintptr_t cacheStreamingCinematAndAudio = 0x477500;
    static constexpr uintptr_t stopStreamingCinemat = 0x477B60;
    static constexpr uintptr_t playStreamingCinemat = 0x478930;
    static constexpr uintptr_t cueStreamingCinemat = 0x4779B0;
    static constexpr uintptr_t cacheStreamingCinemat = 0x476520;
    static constexpr uintptr_t GTFO = 0x2D11C0;
    static constexpr uintptr_t cacheSkeletalAnimationByName = 0x2D9AF0;
    static constexpr uintptr_t enable = 0x2DA340;
    static constexpr uintptr_t setProtonBeamMaxLength = 0x277A50;
    static constexpr uintptr_t detonate = 0x690F0;
    static constexpr uintptr_t attachToActorTag = 0x2BEE80;
    static constexpr uintptr_t setCurrentTeam = 0x15B20;
    static constexpr uintptr_t isTrapDeployed = 0xDE370;
    static constexpr uintptr_t gatherAllDeployedInventoryItems = 0xE4770;
    static constexpr uintptr_t readyInventoryItem = 0xE3F10;
    static constexpr uintptr_t enableInventoryItem = 0xE4530;
    static constexpr uintptr_t isPackOverheated = 0xEA6D0;
    static constexpr uintptr_t slamGoggleLocation = 0xD51D0;
    static constexpr uintptr_t setGoggleLocation = 0xD50E0;
    static constexpr uintptr_t setFacialExpression = 0xCD370;
    static constexpr uintptr_t stopControllingActor = 0x76FD0;
    static constexpr uintptr_t warpToActorSeamless = 0xCDA20;
    static constexpr uintptr_t warpTo = 0x2C4520;
    static constexpr uintptr_t fakePossession = 0xEC1E0;
    static constexpr uintptr_t setFlashlightMode = 0xE5AD0;
    static constexpr uintptr_t toggleflashlight = 0xE3BF0;
    static constexpr uintptr_t commitSuicide = 0xCE560;
    static constexpr uintptr_t setHealth = 0x7A890;
    static constexpr uintptr_t setNothingEquipped = 0xE45A0;
    static constexpr uintptr_t enableAllLights = 0x2E3810;
    static constexpr uintptr_t DanteVMaddExport = 0x2CEC90;
    static constexpr uintptr_t buttonPrompt = 0x2494D0;
    static constexpr uintptr_t setAllowDamageTally = 0x1F8160;
    static constexpr uintptr_t fade = 0x1ECCA0;
    static constexpr uintptr_t displaySplashScreen = 0x1ECD50;
    static constexpr uintptr_t cacheEffect = 0x35A380;
    static constexpr uintptr_t startEffect = 0x35A730;
    static constexpr uintptr_t CreateExplosion = 0x1E9170;
    static constexpr uintptr_t SetGravity = 0x1ECC40;
    static constexpr uintptr_t endGame = 0x1EC500;
    static constexpr uintptr_t AddLight = 0x1ECB20;
    static constexpr uintptr_t CreateActor = 0x2C0D50;
    static constexpr uintptr_t DisplayText = 0x2494A0;
    static constexpr uintptr_t DisplayTextLegacy = 0x2A6C90;
    static constexpr uintptr_t gLocalHero_STEAM = 0x2322AD8;
    static constexpr uintptr_t money = 0x1CADCB0;
    static constexpr uintptr_t static_slewModeChangedCallback = 0x1F9D50;
    static constexpr uintptr_t gMainView = 0xDCF988;
    static constexpr uintptr_t impactCamera = 0x1FF5B0;
    static constexpr uintptr_t shakeCamera = 0x1FF510;
    static constexpr uintptr_t setCameraModeOrbit = 0x1FF800;
    static constexpr uintptr_t resetCamera = 0x1FE7A0;
    static constexpr uintptr_t setAnimation = 0x77440;
    static constexpr uintptr_t startTalking = 0x75BB0;
    static constexpr uintptr_t beginWalkTo = 0x7DBB0;
}
