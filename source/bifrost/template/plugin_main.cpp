
#define BIFROST_IMPLEMENTATION
BIFROST_PLUGIN_INCLUDE

class BIFROST_PLUGIN_NAME final : public BIFROST_PLUGIN_BASE {
 public:
  virtual void SetUp() override {}
  virtual void TearDown() override {}
};

BIFROST_REGISTER_PLUGIN( BIFROST_PLUGIN_NAME )
