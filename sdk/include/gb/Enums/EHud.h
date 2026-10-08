#pragma once

namespace EHUD
{
    enum ETextHudTypes
    {
        eHudMessage_DebugText = 0,
        eHudMessage_GenericText,
        eHudMessage_SpiritGuideUpdated,
        eHudMessage_ObjectivesUpdated,
        eHudMessage_Top = 11,
        eHudMessage_Subtitle,
        eHudMessage_HelpMessage,
        eHudMessage_QuitWarining
    };

    enum eButtonAction
    {
        eButtonAction_WrangleSlam = 0,
        eButtonAction_BlastBeam,
        eButtonAction_ContainBeam,
        eButtonAction_BosonDart,
        eButtonAction_Shotgun,
        eButtonAction_IceBeam,
        eButtonAction_Railgun,
        eButtonAction_RailgunAlt,
        eButtonAction_SlimeGun,
        eButtonAction_TetherGun,
        eButtonAction_Compass,
        eButtonAction_ManualVent = 11,
        eButtonAction_DeployTrap,
        eButtonAction_RecoverTrap,
        eButtonAction_PKEScan,
        eButtonAction_PKEEquip,
        eButtonAction_PKEChirpA,
        eButtonAction_PKEChirpB,
        eButtonAction_PKEChirpC,
        eButtonAction_Medic = 19,
        eButtonAction_InteractNode,
        eButtonAction_Jump,
        eButtonAction_Pole,
        eButtonAction_AdvanceTutorial,
        eButtonAction_Quit = 24,
        eButtonAction_RestartCheckpoint,
        eButtonAction_AccessSpiritGuide,
        eButtonAction_AccessUpgradeMenu,
        eButtonAction_AccessArtifactsMenu,
        eButtonAction_AccessContainmentViewer,
        eButtonAction_NewEquipment,
        eButtonAction_ConfirmQuit,
        eButtonAction_CancelQuit = 32
    };


    enum ETextLegacyPrintTypes
    {
        TEXTL_Default = 1u,
        TEXTL_YesCancelOption = 2,
    };

}