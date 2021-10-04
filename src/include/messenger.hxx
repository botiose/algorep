template <typename T>
void
Messenger::setMessage(const T& code, Message& message) const {
  MessagePassKey passKey;
  int tag = static_cast<int>(getTagFromCode<T>());
  int codeInt = static_cast<int>(code);
  std::shared_ptr<std::string> dataPtr = std::make_shared<std::string>();

  // TODO set to the unique id
  message = Message(passKey, tag, codeInt, 0, dataPtr);
}

template <typename T>
void
Messenger::setMessage(const T& code,
                      const std::string& data,
                      Message& message) const {
  MessagePassKey passKey;
  MessageTag tag = getTagFromCode<T>();
  int codeInt = static_cast<int>(code);
  std::shared_ptr<std::string> dataPtr = std::make_shared<std::string>(data);

  // TODO set to the unique id
  message = Message(passKey, tag, codeInt, 0, dataPtr);
}
