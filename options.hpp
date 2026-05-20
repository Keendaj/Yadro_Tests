#pragma once
#include <unordered_map>
#include <string>
#include <filesystem>

namespace MediaReader
{
    using str = std::string;
    namespace fs = std::filesystem;

    enum class MediaTypes{
        AUDIO,
        VIDEO,
        IMAGE
    };
    using MT = MediaTypes;
    const std::unordered_map<MediaTypes, str> MEDIATYPES_DESCRIPTION = {
        {MT::AUDIO, "audio"},
        {MT::VIDEO, "video"},
        {MT::IMAGE, "images"}
    };
    

    struct Config{
        fs::path scan_catalog;
        int scan_interval = 60;
    };

    const std::unordered_map<str, MediaTypes> EXTENTIONS = {
        {".mp3", MT::AUDIO}, {".wav", MT::AUDIO}, {".ogg", MT::AUDIO}, {".wma", MT::AUDIO}, {".mpc", MT::AUDIO}, {".aac", MT::AUDIO},
        {".mp4", MT::VIDEO}, {".mpeg", MT::VIDEO}, {".wmv", MT::VIDEO}, {".m4v", MT::VIDEO}, {".m2v", MT::VIDEO}, {".flv", MT::VIDEO}, {".webm", MT::VIDEO}, {".mp4", MT::VIDEO}, {".mkv", MT::VIDEO}, {".avi", MT::VIDEO}, {".mpg", MT::VIDEO}, {".mov", MT::VIDEO},
        {".jpg", MT::IMAGE}, {".jpeg", MT::IMAGE}, {".png", MT::IMAGE}, {".gif", MT::IMAGE}, {".bmp", MT::IMAGE}, {".webp", MT::IMAGE}, {".svg", MT::IMAGE}
    };
}