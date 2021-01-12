using BepInEx;
using BepInEx.IL2CPP;
using HarmonyLib;
using UnhollowerBaseLib;
using UnityEngine;

using GameSettingsMenu = JCLABFFHPEO;

namespace GameSettingsUnlocker_BepInEx
{
    [BepInPlugin(id, "Game settings unlocker", "1.0.0")]
    public class Main : BasePlugin
    {
        public const string id = "ru.galster.gamesettingsunlocker";
        public Harmony Harmony { get; } = new Harmony(id);
        public override void Load()
        {
            Harmony.PatchAll();
        }
    }

    [HarmonyPatch(typeof(GameSettingsMenu), nameof(GameSettingsMenu.OnEnable))]
    public static class GameSettingsMenu_OnEnable
    {
        public static void Prefix(ref GameSettingsMenu __instance)
        {
            __instance.HideForOnline = new Il2CppReferenceArray<Transform>(0);
        }
    }
}
