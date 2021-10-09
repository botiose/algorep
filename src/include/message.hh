/**
 * @file   message.hh
 * @author Otiose email
 * @date   Thu Sep 30 11:04:31 2021
 *
 * @brief  Defines the Message struct.
 *
 * This struct encapsulates the tag of a message and its respective code. It
 * is currently the only tag of message supported. You will find the
 * definitions of all supported tags and codes in message-codes.hh.
 *
 */
#pragma once

#include <string>
#include <memory>

#include "message-info.hh"

class MessagePassKey {
  friend class Messenger;

private:
  MessagePassKey() = default;
};

class Message {
public:
  Message() = default;
  Message(const MessagePassKey&,
          const int& tag,
          const int& code,
          const int& id,
          const std::shared_ptr<std::string>& m_data);

  Message&
  operator=(const Message& other);

  MessageTag
  getTag() const;

  int
  getTagInt() const;

  template <typename T>
  T
  getCode() const;

  int
  getCodeInt() const;

  int
  getId() const;

  const std::string&
  getData() const;

  bool
  getIsValid() const;

private:
  int m_tag;
  int m_code; /**< code of the message  */
  int m_id;   /**< unique id of the message */
  std::shared_ptr<std::string> m_data;

  bool m_isValid = false;
};

#include "message.hxx"
