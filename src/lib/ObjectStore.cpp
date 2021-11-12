#include "ObjectStore.hpp"

ObjectStorePtr ObjectStore::__theStore = nullptr;
mutex ObjectStore::__loadingMutex;

