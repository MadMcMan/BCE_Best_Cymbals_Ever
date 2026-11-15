#ifndef CYMBALS_PRESET_POLICY_HPP
#define CYMBALS_PRESET_POLICY_HPP

#include <cctype>
#include <cstdlib>
#include <filesystem>
#include <string>

namespace PresetPolicy {

inline std::string resolvePresetDirectory() {
    const char* envOverride = std::getenv("CYMBALS_PRESET_DIR");
    if (envOverride != nullptr && envOverride[0] != '\0') {
        return std::string(envOverride);
    }

#ifdef _WIN32
    const char* userProfile = std::getenv("USERPROFILE");
    if (userProfile != nullptr && userProfile[0] != '\0') {
        std::filesystem::path up(userProfile);

        std::filesystem::path oldDir = up / "Documents" / "BCE" / "Presets";
        if (std::filesystem::exists(oldDir)) return oldDir.u8string();

        std::filesystem::path docsDir = up / "Documents" / "MadMcMans" / "BCE" / "Presets";
        std::filesystem::path oneDriveDir = up / "OneDrive" / "Documents" / "MadMcMans" / "BCE" / "Presets";
        if (std::filesystem::exists(docsDir)) return docsDir.u8string();
        if (std::filesystem::exists(oneDriveDir)) return oneDriveDir.u8string();
        return docsDir.u8string();
    }

    const char* appData = std::getenv("APPDATA");
    if (appData != nullptr && appData[0] != '\0') {
        return (std::filesystem::path(appData) / "MadMcMans" / "BCE" / "Presets").u8string();
    }
#else
    const char* home = std::getenv("HOME");
    if (home != nullptr && home[0] != '\0') {
        return (std::filesystem::path(home) / "Documents" / "MadMcMans" / "BCE" / "Presets").u8string();
    }
#endif

    return (std::filesystem::current_path() / "MadMcMans" / "BCE" / "Presets").u8string();
}

inline std::string sanitizePresetFileStem(const std::string& name, const char* fallback = "preset") {
    std::string out;
    out.reserve(name.size());
    for (char c : name) {
        const bool ok = std::isalnum((unsigned char)c) || c == '-' || c == '_';
        if (ok) out.push_back(c);
        else if (std::isspace((unsigned char)c)) out.push_back('_');
    }
    if (out.empty()) out = fallback;
    return out;
}

inline std::string canonicalizePresetSavePath(const std::string& requestedPath,
                                              const std::string& metadataName,
                                              const std::string& defaultDir) {
    const std::filesystem::path requested = std::filesystem::u8path(requestedPath);
    const std::filesystem::path directory = requested.has_parent_path()
        ? requested.parent_path()
        : std::filesystem::u8path(defaultDir);

    const std::string baseName = metadataName.empty()
        ? requested.stem().string()
        : metadataName;
    const std::string canonicalStem = sanitizePresetFileStem(baseName);
    return (directory / (canonicalStem + ".json")).u8string();
}

} // namespace PresetPolicy

#endif // CYMBALS_PRESET_POLICY_HPP
