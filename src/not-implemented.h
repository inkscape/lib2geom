class NotImplemented : public std::logic_error {
public:
  NotImplemented() : std::logic_error("method not implemented") {}
};

