#include <Geode/modify/PlayLayer.hpp>
#include "../TimeCounter.hpp"
#include "utils/TimeUtils.hpp"

using namespace geode::prelude;

std::string formatTime(long long time) {
    auto time_h = time / 3600;
    auto time_min = (time - time_h*3600) / 60;
    auto time_sec = (time - time_h*3600 - time_min*60);
    return std::format("{:02}:{:02}:{:02}", time_h, time_min, time_sec);
}

class $modify(PlayLayer) {
    bool init(GJGameLevel *level, bool p1, bool p2) {
        if (!PlayLayer::init(level, p1, p2)) {
            return false;
        }

        TimeCounter::setLevel(level);
        TimeCounter::updateDays();
        // log::debug("PlayLayer_init_updatedays");

        std::string mode = Mod::get()->getSavedValue<std::string>("FirstAtt");
        if (mode != "") {
            TimeCounter::setStartTime(CounterType::Total);
            Mod::get()->setSavedValue("isTotal", true);
            // log::debug("PlayLayer_init_totalstart_isTotaltrue");
        }
        if (mode == "startpos") {
            TimeCounter::setStartTime(CounterType::Startpos);
            Mod::get()->setSavedValue("isStartpos", true);
            Mod::get()->setSavedValue("isNormal", false);
            // log::debug("PlayLayer_init_startposstart_isStartpostrue_isNormalfalse");
        }
        if (mode == "normal") {
            TimeCounter::setStartTime(CounterType::Normal);
            Mod::get()->setSavedValue("isNormal", true);
            Mod::get()->setSavedValue("isStartpos", false);
            // log::debug("PlayLayer_init_normalstart_isNormaltrue_isStartposfalse");
        }
        if (mode == "practice") {
            TimeCounter::setStartTime(CounterType::Practice);
        } 
        // log::debug("PlayLayer_init FirstAtt mode:{}", mode);


        return true;
    }

    void startGame() {
        PlayLayer::startGame();
        TimeCounter::setStartTime(CounterType::NoPause);
        Mod::get()->setSavedValue("CurrentLevel", TimeCounter::levelId);
        // log::debug("PlayLayer_startGame_nopausestart_setcurrentlevel");
    }

    void resetLevel() {
        PlayLayer::resetLevel();
        // log::debug("PlayLayer_resetLevel levelId:{}", TimeCounter::levelId);

        bool startpos = PlayLayer::get()->m_isTestMode;
        bool practice = PlayLayer::get()->m_isPracticeMode;
        // log::debug("PlayLayer_resetLevel startpos and practice get");

        Mod::get()->setSavedValue("SavedTime", getCurrentTimeSeconds());
        Mod::get()->setSavedValue("SavedOnPause", false);
        Mod::get()->setSavedValue("SavedPractice", practice);
        // log::debug("PlayLayer_resetLevel currentTime: {}", getCurrentTimeSeconds());
        // log::debug("PlayLayer_resetLevel recovery values saved");

        if (TimeCounter::levelId != "") {
            bool prevstartpos = Mod::get()->getSavedValue<bool>("isStartpos");
            bool prevnormal = Mod::get()->getSavedValue<bool>("isNormal");
            // log::debug("PlayLayer_resetLevel startpos:{}, practice:{}, prevstartpos:{}, prevnormal:{}, id:{}", startpos, practice, prevstartpos, prevnormal, TimeCounter::levelId);
            if (!Mod::get()->getSavedValue<bool>("isTotal")) {
                TimeCounter::setStartTime(CounterType::Total);
                Mod::get()->setSavedValue("isTotal", true);
                // log::debug("PlayLayer_resetLevel_totalstart_isTotaltrue");  
            }
            if (Mod::get()->getSavedValue<bool>("isComplete")) {
                TimeCounter::setStartTime(CounterType::NoPause);
                Mod::get()->setSavedValue("isComplete", false);
                // log::debug("PlayLayer_resetLevel_nopausestart_isCompletefalse");  
            }

            if (startpos) {
                if (!prevstartpos) {
                    TimeCounter::setStartTime(CounterType::Startpos);
                    Mod::get()->setSavedValue("isStartpos", true);
                    // log::debug("PlayLayer_resetLevel_startposstart_isStartpostrue");
                }
            }
            else {
                if (prevstartpos) {
                    TimeCounter::updateTotalTime(CounterType::Startpos);
                    Mod::get()->setSavedValue("isStartpos", false);
                    // log::debug("PlayLayer_resetLevel_startposstop_isStartposfalse");
                }
            }
            if (!startpos && !practice) {
                if (!prevnormal) {
                    TimeCounter::setStartTime(CounterType::Normal);
                    Mod::get()->setSavedValue("isNormal", true);
                    // log::debug("PlayLayer_resetLevel_normalstart_isNormaltrue");
                }
            }
            else {
                if (prevnormal) {
                    TimeCounter::updateTotalTime(CounterType::Normal);
                    Mod::get()->setSavedValue("isNormal", false);
                    // log::debug("PlayLayer_resetLevel_normalstop_isNormalfalse");
                }
            }
        } else {
            // log::debug("PlayLayer_resetLevel SKIP");
            std::string mode = "";
            // log::debug("PlayLayer_resetLevel startpos:{}, practice:{}", startpos, practice);
            if (startpos) {mode = "startpos";} else if (!practice) {mode = "normal";}
            if (practice) {mode = "practice";} 
            Mod::get()->setSavedValue("FirstAtt", mode);
        }
    }
    

    void levelComplete() {
        PlayLayer::levelComplete();
        TimeCounter::updateTotalTime(CounterType::NoPause);

        Mod::get()->setSavedValue("isComplete", true);
        Mod::get()->setSavedValue("SavedTime", getCurrentTimeSeconds());
        Mod::get()->setSavedValue("SavedOnPause", true);
        Mod::get()->setSavedValue("SavedPractice", PlayLayer::get()->m_isPracticeMode);
        // log::debug("PlayLayer_levelComplete_nopausestop");
    }

    void togglePracticeMode(bool p0) {
        PlayLayer::togglePracticeMode(p0);
        if (p0) {
            TimeCounter::setStartTime(CounterType::Practice);
            // log::debug("PlayLayer_togglePracticeMode_practicestart");
            if (Mod::get()->getSavedValue<bool>("isNormal")) {
                TimeCounter::updateTotalTime(CounterType::Normal);
                Mod::get()->setSavedValue("isNormal", false);
                // log::debug("PlayLayer_togglePracticeMode_normalstop_isNormalfalse");
            }
        }
        else {
            TimeCounter::updateTotalTime(CounterType::Practice);
            // log::debug("PlayLayer_togglePracticeMode_practicestop");
        }
    }

    void onQuit() {
        PlayLayer::onQuit();
        TimeCounter::updateTotalTime(CounterType::Total);
        bool practice = PlayLayer::get()->m_isPracticeMode;
        bool startpos = PlayLayer::get()->m_isTestMode;
        if (practice) {TimeCounter::updateTotalTime(CounterType::Practice);}
        if (startpos) {TimeCounter::updateTotalTime(CounterType::Startpos);}
        if (!practice && !startpos) {TimeCounter::updateTotalTime(CounterType::Normal);}
        TimeCounter::updateDays();
        std::string empty = "";
        Mod::get()->setSavedValue("CurrentLevel", empty);
        Mod::get()->setSavedValue("FirstAtt", empty);
        Mod::get()->setSavedValue("isTotal", false);
        Mod::get()->setSavedValue("isComplete", false);
        Mod::get()->setSavedValue("isStartpos", false);
        Mod::get()->setSavedValue("isNormal", false);
        // log::debug("PlayLayer_onQuit_withpauseallstop_isTotalisCompleteisStartposisNormalfalse_FirstAttnone");
        // log::debug("PlayLayer_onQuit practice:{}, startpos:{}", practice, startpos);
        // log::info("{} attemptTime: {}", this->m_level->m_levelName, this->m_level->m_attemptTime.value());
        // log::info("{} workingTime: {}", this->m_level->m_levelName, this->m_level->m_workingTime);
        // log::info("{} workingTime2: {}", this->m_level->m_levelName, this->m_level->m_workingTime2);
        // logging

        log::info("*LEVEL NAME: {}*", TimeCounter::levelName);
        log::info(" {} - Total Playtime", formatTime(TimeCounter::getTotalTime(CounterType::Total)));
        log::info(" {} - Normal Mode", formatTime(TimeCounter::getTotalTime(CounterType::Normal)));
        log::info(" {} - Practice Mode", formatTime(TimeCounter::getTotalTime(CounterType::Practice)));
        log::info(" {} - Start Position", formatTime(TimeCounter::getTotalTime(CounterType::Startpos)));
        log::info(" {} - Without Pauses", formatTime(TimeCounter::getTotalTime(CounterType::NoPause)));
        log::info(" Days Entering Level - {}", TimeCounter::getTotalDays());

        TimeCounter::levelId = "";
    }

};