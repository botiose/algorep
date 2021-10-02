#include "message.hh"
#include "message-info.hh"

MessageTag
Message::getTag() const {
  return static_cast<MessageTag>(m_tag);
}

int
Message::getTagInt() const {
  return m_tag;
}

int
Message::getId() const {
  return m_id;
}

std::string
Message::getCommand() const {
  return m_command;
}

bool
Message::getIsValid() const {
  return m_isValid;
}

void
Message::setTag(const MessagePassKey&, const int& tag) {
  m_tag = tag;
}

void
Message::setId(const MessagePassKey&, const int& id) {
  m_id = id;
}

void
Message::setCommand(const MessagePassKey&, const std::string& command) {
  m_command = command;
}

void
Message::setToValid(const MessagePassKey&) {
  m_isValid = true;
}
