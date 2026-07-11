#pragma once

#include "manager.h"

#ifndef NOMINMAX
#define NOMINMAX
#endif
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

class SaveManager : public Manager<SaveManager>
{
	friend class Manager<SaveManager>;

public:
	struct LevelRecord
	{
		bool cleared = false;
		int best_home_hp = 0;
		int best_coin = 0;
		int clear_count = 0;
	};

	bool initialize(const std::string& path)
	{
		save_path = path;
		try_load_sqlite();
		if (sqlite_handle)
			load_sqlite_data();
		else
			load_fallback_file();
		select_slot(1);
		return true;
	}

	void select_slot(int slot)
	{
		active_slot = clamp_slot(slot);
		ensure_slot(active_slot);
	}

	int get_active_slot() const
	{
		return active_slot;
	}

	int get_slot_count() const
	{
		return 3;
	}

	bool has_slot_data(int slot) const
	{
		slot = clamp_slot(slot);
		const SlotData& data = slots[slot];
		if (data.touched || data.highest_unlocked_level > 1)
			return true;

		for (const LevelRecord& record : data.records)
			if (record.cleared || record.clear_count > 0)
				return true;
		return false;
	}

	int get_slot_highest_unlocked_level(int slot) const
	{
		slot = clamp_slot(slot);
		return (std::max)(1, slots[slot].highest_unlocked_level);
	}

	int get_slot_clear_count(int slot) const
	{
		slot = clamp_slot(slot);
		int count = 0;
		for (const LevelRecord& record : slots[slot].records)
			if (record.cleared)
				count++;
		return count;
	}

	int get_highest_unlocked_level() const
	{
		return get_slot_highest_unlocked_level(active_slot);
	}

	bool is_level_unlocked(int level_id) const
	{
		return level_id <= get_highest_unlocked_level();
	}

	LevelRecord get_level_record(int level_id) const
	{
		const SlotData& data = slots[active_slot];
		if (level_id <= 0 || level_id >= (int)data.records.size())
			return LevelRecord();
		return data.records[level_id];
	}

	void complete_level(int level_id, int home_hp, int coin, int max_level)
	{
		ensure_slot(active_slot);
		SlotData& data = slots[active_slot];
		ensure_record(data, level_id);

		LevelRecord& record = data.records[level_id];
		record.cleared = true;
		record.best_home_hp = (std::max)(record.best_home_hp, home_hp);
		record.best_coin = (std::max)(record.best_coin, coin);
		record.clear_count++;
		data.touched = true;

		if (level_id >= data.highest_unlocked_level && level_id < max_level)
			data.highest_unlocked_level = level_id + 1;

		if (sqlite_handle)
			save_sqlite_data(active_slot, level_id);
		else
			save_fallback_file();
	}

	bool uses_sqlite_runtime() const
	{
		return sqlite_handle != nullptr;
	}

private:
	struct SlotData
	{
		int highest_unlocked_level = 1;
		bool touched = false;
		std::vector<LevelRecord> records;

		SlotData()
		{
			records.resize(16);
		}
	};

	SaveManager()
	{
		slots.resize(4);
	}

	~SaveManager()
	{
		if (sqlite_handle)
			FreeLibrary(sqlite_handle);
	}

private:
	typedef int(__cdecl* sqlite3_open_fn)(const char*, void**);
	typedef int(__cdecl* sqlite3_close_fn)(void*);
	typedef int(__cdecl* sqlite3_exec_fn)(void*, const char*, int(*)(void*, int, char**, char**), void*, char**);

	std::string save_path = "save.db";
	int active_slot = 1;
	std::vector<SlotData> slots;

	HMODULE sqlite_handle = nullptr;
	sqlite3_open_fn sqlite3_open_ptr = nullptr;
	sqlite3_close_fn sqlite3_close_ptr = nullptr;
	sqlite3_exec_fn sqlite3_exec_ptr = nullptr;

private:
	int clamp_slot(int slot) const
	{
		return (std::max)(1, (std::min)(slot, get_slot_count()));
	}

	void ensure_slot(int slot)
	{
		if (slot >= (int)slots.size())
			slots.resize(slot + 1);
	}

	void ensure_record(SlotData& data, int level_id)
	{
		if (level_id >= (int)data.records.size())
			data.records.resize(level_id + 1);
	}

	void try_load_sqlite()
	{
		sqlite_handle = LoadLibraryA("sqlite3.dll");
		if (!sqlite_handle) return;

		sqlite3_open_ptr = (sqlite3_open_fn)GetProcAddress(sqlite_handle, "sqlite3_open");
		sqlite3_close_ptr = (sqlite3_close_fn)GetProcAddress(sqlite_handle, "sqlite3_close");
		sqlite3_exec_ptr = (sqlite3_exec_fn)GetProcAddress(sqlite_handle, "sqlite3_exec");

		if (!sqlite3_open_ptr || !sqlite3_close_ptr || !sqlite3_exec_ptr)
		{
			FreeLibrary(sqlite_handle);
			sqlite_handle = nullptr;
			return;
		}

		void* db = nullptr;
		if (sqlite3_open_ptr(save_path.c_str(), &db) == 0 && db)
		{
			sqlite3_exec_ptr(db, "CREATE TABLE IF NOT EXISTS profile_slot(slot_id INTEGER PRIMARY KEY, highest_unlocked_level INTEGER, updated_at TEXT);", nullptr, nullptr, nullptr);
			sqlite3_exec_ptr(db, "CREATE TABLE IF NOT EXISTS level_record_slot(slot_id INTEGER, level_id INTEGER, cleared INTEGER, best_home_hp INTEGER, best_coin INTEGER, clear_count INTEGER, updated_at TEXT, PRIMARY KEY(slot_id, level_id));", nullptr, nullptr, nullptr);
			for (int slot = 1; slot <= get_slot_count(); slot++)
			{
				std::stringstream sql;
				sql << "INSERT OR IGNORE INTO profile_slot(slot_id, highest_unlocked_level, updated_at) VALUES("
					<< slot << ", 1, datetime('now'));";
				sqlite3_exec_ptr(db, sql.str().c_str(), nullptr, nullptr, nullptr);
			}
			sqlite3_close_ptr(db);
		}
	}

	static int profile_callback(void* data, int argc, char** argv, char** col)
	{
		SaveManager* self = (SaveManager*)data;
		if (argc < 2 || !argv[0])
			return 0;

		int slot = self->clamp_slot(std::atoi(argv[0]));
		self->ensure_slot(slot);
		self->slots[slot].highest_unlocked_level = argv[1] ? (std::max)(1, std::atoi(argv[1])) : 1;
		return 0;
	}

	static int record_callback(void* data, int argc, char** argv, char** col)
	{
		SaveManager* self = (SaveManager*)data;
		if (argc < 6 || !argv[0] || !argv[1])
			return 0;

		int slot = self->clamp_slot(std::atoi(argv[0]));
		int id = std::atoi(argv[1]);
		self->ensure_slot(slot);
		SlotData& slot_data = self->slots[slot];
		self->ensure_record(slot_data, id);

		LevelRecord& record = slot_data.records[id];
		record.cleared = argv[2] ? std::atoi(argv[2]) != 0 : false;
		record.best_home_hp = argv[3] ? std::atoi(argv[3]) : 0;
		record.best_coin = argv[4] ? std::atoi(argv[4]) : 0;
		record.clear_count = argv[5] ? std::atoi(argv[5]) : 0;
		slot_data.touched = true;
		return 0;
	}

	void load_sqlite_data()
	{
		void* db = nullptr;
		if (sqlite3_open_ptr(save_path.c_str(), &db) != 0 || !db)
			return;

		sqlite3_exec_ptr(db, "SELECT slot_id, highest_unlocked_level FROM profile_slot;", profile_callback, this, nullptr);
		sqlite3_exec_ptr(db, "SELECT slot_id, level_id, cleared, best_home_hp, best_coin, clear_count FROM level_record_slot;", record_callback, this, nullptr);
		sqlite3_close_ptr(db);
	}

	void save_sqlite_data(int slot, int level_id)
	{
		void* db = nullptr;
		if (sqlite3_open_ptr(save_path.c_str(), &db) != 0 || !db)
			return;

		const SlotData& slot_data = slots[slot];
		const LevelRecord& record = slot_data.records[level_id];
		std::stringstream profile_sql;
		profile_sql << "INSERT OR REPLACE INTO profile_slot(slot_id, highest_unlocked_level, updated_at) VALUES("
			<< slot << ", " << slot_data.highest_unlocked_level << ", datetime('now'));";
		sqlite3_exec_ptr(db, profile_sql.str().c_str(), nullptr, nullptr, nullptr);

		std::stringstream record_sql;
		record_sql << "INSERT OR REPLACE INTO level_record_slot(slot_id, level_id, cleared, best_home_hp, best_coin, clear_count, updated_at) VALUES("
			<< slot << ", " << level_id << ", " << (record.cleared ? 1 : 0) << ", " << record.best_home_hp << ", "
			<< record.best_coin << ", " << record.clear_count << ", datetime('now'));";
		sqlite3_exec_ptr(db, record_sql.str().c_str(), nullptr, nullptr, nullptr);
		sqlite3_close_ptr(db);
	}

	void load_fallback_file()
	{
		std::ifstream file(save_path);
		if (!file.good())
			return;

		std::string token;
		while (file >> token)
		{
			if (token == "slot")
			{
				int slot = 1;
				std::string field;
				file >> slot >> field;
				slot = clamp_slot(slot);
				ensure_slot(slot);
				if (field == "highest")
				{
					file >> slots[slot].highest_unlocked_level;
					slots[slot].highest_unlocked_level = (std::max)(1, slots[slot].highest_unlocked_level);
				}
				else if (field == "level")
				{
					int id = 0;
					LevelRecord record;
					file >> id >> record.cleared >> record.best_home_hp >> record.best_coin >> record.clear_count;
					ensure_record(slots[slot], id);
					slots[slot].records[id] = record;
					slots[slot].touched = true;
				}
			}
			else if (token == "highest")
			{
				file >> slots[1].highest_unlocked_level;
				slots[1].highest_unlocked_level = (std::max)(1, slots[1].highest_unlocked_level);
			}
			else if (token == "level")
			{
				int id = 0;
				LevelRecord record;
				file >> id >> record.cleared >> record.best_home_hp >> record.best_coin >> record.clear_count;
				ensure_record(slots[1], id);
				slots[1].records[id] = record;
				slots[1].touched = true;
			}
		}
	}

	void save_fallback_file()
	{
		std::ofstream file(save_path, std::ios::trunc);
		if (!file.good()) return;

		for (int slot = 1; slot <= get_slot_count(); slot++)
		{
			const SlotData& slot_data = slots[slot];
			file << "slot " << slot << " highest " << slot_data.highest_unlocked_level << "\n";
			for (int level = 1; level < (int)slot_data.records.size(); level++)
			{
				const LevelRecord& record = slot_data.records[level];
				if (!record.cleared && record.clear_count == 0) continue;
				file << "slot " << slot << " level " << level << " " << record.cleared << " "
					<< record.best_home_hp << " " << record.best_coin << " " << record.clear_count << "\n";
			}
		}
	}
};
