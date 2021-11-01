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
  /**
   * @brief Default constructor for the Message class
   *
   */
  Message() = default;

  /**
   * @brief Second constructor for the Message class
   *
   * This consturctor is only meant to be used by the Messenger class. This
   * is enforced through the pass key idiom.
   *
   * @param[in] tag message's tag
   * @param[in] code message's code
   * @param[in] id message's id
   * @param[in] data message's arbitrary data field
   */
  Message(const MessagePassKey&,
          const int& tag,
          const int& code,
          const int& id,
          const std::shared_ptr<std::string>& data);

  /**
   * @brief Copy constructor for the Message class.
   *
   * @param other message instance from which to copy
   *
   * @return contructed Message
   */
  Message&
  operator=(const Message& other);

  /**
   * @brief Gets the message tag.
   *
   * @return message tag
   */
  MessageTag
  getTag() const;

  /**
   * @brief Gets the message tag casted to an int.
   *
   * @return int message tag
   */
  int
  getTagInt() const;

  /**
   * @brief Gets the message's code
   *
   * @return message code
   */
  template <typename T>
  T
  getCode() const;

  /**
   * @brief Gets the message's code casted to an int.
   *
   * @return int message code
   */
  int
  getCodeInt() const;

  /**
   * @brief Gets the message's id.
   *
   * @return message's id
   */
  int
  getId() const;

  /**
   * @brief Gets the data field of the message.
   *
   * @return message's data field
   */
  const std::string&
  getData() const;

  /**
   * @brief Sets the data field of the message.
   *
   */
  void
  setData(const std::string&);

  /**
   * @brief returns whether the message was properly initialized.
   *
   * This is set to true of the non-defaulted constructor was used for
   * constructing this message.
   *
   * @return
   */
  bool
  getIsValid() const;

private:
  int m_tag;
  int m_code;
  int m_id;
  std::shared_ptr<std::string> m_data;

  bool m_isValid = false;
};

#include "message.hxx"
