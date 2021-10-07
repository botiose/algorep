template <typename T>
void
Messenger::setMessage(const T& code,
                      const std::string& data,
                      Message& message) const {
  MessagePassKey passKey;
  int tag = static_cast<int>(getTagFromCode<T>());
  int codeInt = static_cast<int>(code);
  int id;
  this->generateUniqueId(m_rank, id);
  std::shared_ptr<std::string> dataPtr = std::make_shared<std::string>(data);

  message = Message(passKey, tag, codeInt, id, dataPtr);
}

template <typename T>
void
Messenger::setMessage(const T& code, Message& message) const {
  setMessage<T>(code, "", message);
}
