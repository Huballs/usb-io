//
// Created by hugo on 19.07.25.
//

#ifndef FS_HPP
#define FS_HPP

#include <filesystem>
#include <fstream>
#include <string>
#include <utility>
#include <vector>
#include <memory>
#include <exception>

namespace fs {

//class Directory;

//using item_t = std::variant<std::filesystem::path, Directory>;

enum class item_t {
    FILE
    , DIR
};

class Fs;
class ItemObj;

using Item = std::shared_ptr<ItemObj>;
using ItemConst = std::shared_ptr<ItemObj>;

class ItemObj {

public:

    using contents_t = std::vector<Item>;

    std::string name() const noexcept {
        return m_path.filename();
    }

    std::string path() const noexcept {
        return m_path;
    }

    item_t type() const noexcept {
        return m_type;
    }

    ItemConst parent() noexcept {
        return m_parent;
    }

    std::string read_file() const noexcept {

        if (m_type != item_t::FILE)
            return "";

        std::fstream s(m_path);
        std::stringstream ss{};
        ss << s.rdbuf();
        return ss.str();
    }

    void write_file(std::string_view text) const {
        std::fstream file(m_path);

        if (!file.is_open()) {
            throw std::runtime_error("can't open file");
        }

        file << text;
    }

    const contents_t& contents() const noexcept {
        return m_contents;
    }

private:

    friend Fs;

    std::filesystem::path m_path;
    item_t m_type;
    contents_t m_contents;
    Item m_parent;
};

class Fs {

public:
    Fs(std::string_view parent_dir, std::optional<std::string> extension = std::nullopt)
        : m_parent_dir(parent_dir), m_extension(std::move(extension)) {}

    void fill() noexcept {
        m_top_level = std::make_shared<ItemObj>();
        m_top_level->m_path = m_parent_dir;
        walk_dir(m_top_level);
    }

    ItemConst top_level() const noexcept {
        return m_top_level;
    }

private:

    void walk_dir(Item parent) noexcept {

        for (auto& it : std::filesystem::directory_iterator(parent->m_path)) {

            auto item = std::make_shared<ItemObj>();
            item->m_path = it.path();
            item->m_parent = parent;

            if (it.is_directory()) {
                item->m_type = item_t::DIR;
                parent->m_contents.push_back(item);
                walk_dir(item);
            } else {
                item->m_type = item_t::FILE;
                if (m_extension && (it.path().extension() == m_extension.value())) {
                    parent->m_contents.push_back(std::move(item));
                } else if (!m_extension) {
                    parent->m_contents.push_back(std::move(item));
                }
            }
        }
    }

    std::filesystem::path m_parent_dir;
    Item m_top_level;
    std::optional<std::string> m_extension;
};

}

#endif //FS_HPP
