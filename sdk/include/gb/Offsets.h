// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
#pragma once
// Script-native and object addresses in ghost.exe, ghost-relative, { Steam, Epic }. Most are

#include <cstdint>
#include "gb/Build.h"

namespace Offsets
{
    static constexpr GbAddr gGameBase = { 0x23229C0, 0x227A770 };
    static constexpr GbAddr gGame = { 0xDCF680, 0xD2B680 };
    static constexpr GbAddr getOrient = { 0x2BDB50, 0x2BD960 };
    static constexpr GbAddr enableActorsInsideMe = { 0x4A67F0, 0x4A6480 };
    static constexpr GbAddr findActorByName = { 0x2DB760, 0x2DB570 };
    static constexpr GbAddr exportGlobalVariable = { 0x2CED00, 0x2CEB10 };
    static constexpr GbAddr startPreparedTalking = { 0x75A00, 0x75A00 };
    static constexpr GbAddr setGhostbusterHeatlhState = { 0xD0280, 0xD0280 };
    static constexpr GbAddr flinch = { 0xD18B0, 0xD18B0 };
    static constexpr GbAddr Singleton_getRoom = { 0x2BDBD0, 0x2BD9E0 };
    static constexpr GbAddr setMusic = { 0x411FE0, 0x411DF0 };
    static constexpr GbAddr hideHack = { 0xEE2D0, 0xEE2D0 };
    static constexpr GbAddr removeSlimeDecals = { 0x30D620, 0x30D430 };
    static constexpr GbAddr dbNarrativeStop = { 0x1F8410, 0x1F8350 };
    static constexpr GbAddr dbNarrative = { 0x1ECEB0, 0x1ECEB0 };
    static constexpr GbAddr findDBEntry = { 0x2CF5E0, 0x2CF3F0 };
    static constexpr GbAddr DanteVMInstance = { 0x222D41C, 0x21851CC };
    static constexpr GbAddr Singleton_newActor = { 0x2C0D50, 0x2C0B60 };
    static constexpr GbAddr die = { 0x3B27B0, 0x3B25C0 };
    static constexpr GbAddr blockHeroMovement = { 0xED660, 0xED660 };
    static constexpr GbAddr toggleHuntMode = { 0xED480, 0xED480 };
    static constexpr GbAddr enableProtonTorpedo = { 0xEDE30, 0xEDE30 };
    static constexpr GbAddr setCommandCrossBeam = { 0xEC640, 0xEC640 };
    static constexpr GbAddr startFakePackOverheat = { 0xED750, 0xED750 };
    static constexpr GbAddr letterbox = { 0x2D87A0, 0x2D85B0 };
    static constexpr GbAddr queueVideo = { 0x2D87E0, 0x2D85F0 };
    static constexpr GbAddr setMovieCaptureEnable = { 0x2D8850, 0x2D8660 };
    static constexpr GbAddr allowEnemyAttack = { 0x2D8420, 0x2D8230 };
    static constexpr GbAddr allowHeroControls = { 0x2D8440, 0x2D8250 };
    static constexpr GbAddr allowHeroDamage = { 0x2D8460, 0x2D8270 };
    static constexpr GbAddr play = { 0x1C3F0, 0x1C3F0 };
    static constexpr GbAddr setCameraPathActor = { 0x1FF760, 0x1FF590 };
    static constexpr GbAddr shatter = { 0x49EC70, 0x49E900 };
    static constexpr GbAddr setSimEnable = { 0x89940, 0x89940 };
    static constexpr GbAddr loadCheckpoint = { 0x1F81F0, 0x1F8130 };
    static constexpr GbAddr setCurrentObjective = { 0x1F8200, 0x1F8140 };
    static constexpr GbAddr toggleReviveMode = { 0xD0EE0, 0xD0EE0 };
    static constexpr GbAddr chainToLevel = { 0x1EF700, 0x1EF6C0 };
    static constexpr GbAddr transferHeroshipTo = { 0xD81A0, 0xD81A0 };
    static constexpr GbAddr slimeMe = { 0xD0F50, 0xD0F50 };
    static constexpr GbAddr knockBack = { 0xED100, 0xED100 };
    static constexpr GbAddr pretendToDrive = { 0xEAC40, 0xEAC40 };
    static constexpr GbAddr mountProtonPack = { 0xE4640, 0xE4640 };
    static constexpr GbAddr fakeFireProtonGun = { 0xE9060, 0xE9060 };
    static constexpr GbAddr cacheRappel = { 0xE1D70, 0xE1D70 };
    static constexpr GbAddr forceDeployTrap = { 0xE4C80, 0xE4C80 };
    static constexpr GbAddr setRappelModeEnable = { 0xE1F70, 0xE1F70 };
    static constexpr GbAddr startRappelSwing = { 0xE21E0, 0xE21E0 };
    static constexpr GbAddr isDead = { 0x7B170, 0x7B170 };
    static constexpr GbAddr cacheStreamingCinematAndAudio = { 0x477500, 0x477190 };
    static constexpr GbAddr stopStreamingCinemat = { 0x477B60, 0x4777F0 };
    static constexpr GbAddr playStreamingCinemat = { 0x478930, 0x4785C0 };
    static constexpr GbAddr cueStreamingCinemat = { 0x4779B0, 0x477640 };
    static constexpr GbAddr cacheStreamingCinemat = { 0x476520, 0x4761B0 };
    static constexpr GbAddr GTFO = { 0x2D11C0, 0x2D0FD0 };
    static constexpr GbAddr cacheSkeletalAnimationByName = { 0x2D9AF0, 0x2D9900 };
    static constexpr GbAddr enable = { 0x2DA340, 0x2DA150 };
    static constexpr GbAddr setProtonBeamMaxLength = { 0x277A50, 0x277870 };
    static constexpr GbAddr detonate = { 0x690F0, 0x690F0 };
    static constexpr GbAddr attachToActorTag = { 0x2BEE80, 0x2BEC90 };
    static constexpr GbAddr setCurrentTeam = { 0x15B20, 0x15B20 };
    static constexpr GbAddr isTrapDeployed = { 0xDE370, 0xDE370 };
    static constexpr GbAddr gatherAllDeployedInventoryItems = { 0xE4770, 0xE4770 };
    static constexpr GbAddr readyInventoryItem = { 0xE3F10, 0xE3F10 };
    static constexpr GbAddr enableInventoryItem = { 0xE4530, 0xE4530 };
    static constexpr GbAddr isPackOverheated = { 0xEA6D0, 0xEA6D0 };
    static constexpr GbAddr slamGoggleLocation = { 0xD51D0, 0xD51D0 };
    static constexpr GbAddr setGoggleLocation = { 0xD50E0, 0xD50E0 };
    static constexpr GbAddr setFacialExpression = { 0xCD370, 0xCD370 };
    static constexpr GbAddr stopControllingActor = { 0x76FD0, 0x76FD0 };
    static constexpr GbAddr warpToActorSeamless = { 0xCDA20, 0xCDA20 };
    static constexpr GbAddr warpTo = { 0x2C4520, 0x2C4330 };
    static constexpr GbAddr fakePossession = { 0xEC1E0, 0xEC1E0 };
    static constexpr GbAddr setFlashlightMode = { 0xE5AD0, 0xE5AD0 };
    static constexpr GbAddr toggleflashlight = { 0xE3BF0, 0xE3BF0 };
    static constexpr GbAddr commitSuicide = { 0xCE560, 0xCE560 };
    static constexpr GbAddr setHealth = { 0x7A890, 0x7A890 };
    static constexpr GbAddr setNothingEquipped = { 0xE45A0, 0xE45A0 };
    static constexpr GbAddr enableAllLights = { 0x2E3810, 0x2E3620 };
    static constexpr GbAddr DanteVMaddExport = { 0x2CEC90, 0x2CEAA0 };
    static constexpr GbAddr buttonPrompt = { 0x2494D0, 0x2492F0 };
    static constexpr GbAddr setAllowDamageTally = { 0x1F8160, 0x1F80A0 };
    static constexpr GbAddr fade = { 0x1ECCA0, 0x1ECCA0 };
    static constexpr GbAddr displaySplashScreen = { 0x1ECD50, 0x1ECD50 };
    static constexpr GbAddr cacheEffect = { 0x35A380, 0x35A190 };
    static constexpr GbAddr startEffect = { 0x35A730, 0x35A540 };
    static constexpr GbAddr CreateExplosion = { 0x1E9170, 0x1E9170 };
    static constexpr GbAddr SetGravity = { 0x1ECC40, 0x1ECC40 };
    static constexpr GbAddr endGame = { 0x1EC500, 0x1EC500 };
    static constexpr GbAddr AddLight = { 0x1ECB20, 0x1ECB20 };
    static constexpr GbAddr CreateActor = { 0x2C0D50, 0x2C0B60 };
    static constexpr GbAddr DisplayText = { 0x2494A0, 0x2492C0 };
    static constexpr GbAddr DisplayTextLegacy = { 0x2A6C90, 0x2A6AB0 };
    static constexpr GbAddr gLocalHero_STEAM = { 0x2322AD8, 0x227A888 };
    static constexpr GbAddr money = { 0x1CADCB0, 0x0 };   // Epic not proven: no code reads it
    static constexpr GbAddr static_slewModeChangedCallback = { 0x1F9D50, 0x1F9C90 };
    static constexpr GbAddr gMainView = { 0xDCF988, 0xD2B858 };
    static constexpr GbAddr impactCamera = { 0x1FF5B0, 0x1FF3E0 };
    static constexpr GbAddr shakeCamera = { 0x1FF510, 0x1FF340 };
    static constexpr GbAddr setCameraModeOrbit = { 0x1FF800, 0x1FF630 };
    static constexpr GbAddr resetCamera = { 0x1FE7A0, 0x1FE5D0 };
    static constexpr GbAddr setAnimation = { 0x77440, 0x77440 };
    static constexpr GbAddr startTalking = { 0x75BB0, 0x75BB0 };
    static constexpr GbAddr beginWalkTo = { 0x7DBB0, 0x7DBB0 };
}
