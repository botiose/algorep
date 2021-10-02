template <typename T>
T
Message::getCode() const {
  return static_cast<T>(m_code);
}

template <typename T>
void
Message::setCode(const MessagePassKey&, const T& code) {
  m_code = static_cast<int>(code);
}
