using BepInEx;
using BepInEx.IL2CPP;
using HarmonyLib;
using UnhollowerBaseLib;
using UnityEngine;

namespace GameSettingsUnlocker_BepInEx
{
    [BepInPlugin(Id, "Game settings unlocker", "1.0.1")]
    public class MainPlugin : BasePlugin
    {
        public const string Id = "ru.galster.gamesettingsunlocker";
        private Harmony Harmony { get; } = new Harmony(Id);
        public override void Load()
        {
            Harmony.PatchAll();
        }
    }

    [HarmonyPatch(typeof(GameSettingMenu), nameof(GameSettingMenu.OnEnable))]
    public static class GameSettingsMenu_OnEnable
    {
        public static void Prefix(ref GameSettingMenu __instance)
        {
            __instance.HideForOnline = new Il2CppReferenceArray<Transform>(0);
        }
    }
}
