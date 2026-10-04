#pragma once

#include <Geode/Enums.hpp>
#include <Geode/ui/Popup.hpp>
#include <Geode/ui/TextInput.hpp>
#include <Geode/ui/Label.hpp>
#include <Geode/ui/LoadingSpinner.hpp>

#include <Geode/binding/CommentUploadDelegate.hpp>

#include <string>

class ReplyPopup : public geode::Popup {
public:
    static ReplyPopup* create(int levelID, const std::string& username);
protected:
    friend class ReplyUploadPopup;

    int m_levelID;
    std::string m_username;
    geode::TextInput* m_textInput;
    geode::Label* m_charLabel;

    bool init(int levelID, const std::string& username);
    int getRemainingChars();
};

class ReplyUploadPopup : public geode::Popup, public CommentUploadDelegate {
public:
    static ReplyUploadPopup* create(int levelID, const std::string& comment, ReplyPopup* replyPopup);

    void commentUploadFinished(int parentID) override;
    void commentUploadFailed(int parentID, CommentError errorType) override;
protected:
    ReplyPopup* m_replyPopup;
    bool m_finished = false;

    geode::LoadingSpinner* m_loadingSpinner;
    geode::Label* m_label;

    bool init(int levelID, const std::string& comment, ReplyPopup* replyPopup);
    void onClose(CCObject* sender) override;
private:
    ~ReplyUploadPopup();
};
