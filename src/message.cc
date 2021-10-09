#include "message.hh"
#include "message-info.hh"

Message::Message(const MessagePassKey&,
                 const int& tag,
                 const int& code,
                 const int& id,
                 const std::shared_ptr<std::string>& data)
    : m_tag(tag), m_code(code), m_id(id), m_data(data), m_isValid(true) {
}

Message&
Message::operator=(const Message& other) {
  m_tag = other.m_tag;
  m_code = other.m_code;
  m_id = other.m_id;
  m_data = other.m_data;
  m_isValid = other.m_isValid;

  return *this;
}

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

const std::string&
Message::getData() const {
  return *m_data.get();
}

bool
Message::getIsValid() const {
  return m_isValid;
}

int
Message::getCodeInt() const {
  return m_code;
}
