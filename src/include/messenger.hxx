template <typename T>
void
Messenger::setMessage(const T& code, Message& message) const {
  MessagePassKey passKey;
  MessageTag tag = getTagFromCode<T>();
  message.setTag(passKey, static_cast<int>(tag));
  message.setCode<T>(passKey, code);
  message.setId(passKey, 0); // TODO set to the unique id

  message.setToValid(passKey);
}

template <typename T>
void
Messenger::setMessage(const T& code,
                      const std::string& command,
                      Message& message) const {
  MessagePassKey passKey;

  MessageTag tag = getTagFromCode<T>();
  message.setTag(passKey, static_cast<int>(tag));
  message.setCode<T>(passKey, code);
  message.setId(passKey, 0); // TODO set to the unique id
  message.setCommand(passKey, command);

  message.setToValid(passKey);
}
