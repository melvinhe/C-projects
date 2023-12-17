#include "simple_kvstore.hpp"

bool SimpleKvStore::Get(const GetRequest* req, GetResponse* res) {
  // TODO (Part A, Steps 1 and 2): IMPLEMENT
  std::unique_lock<std::mutex> lock(kvStore_mutex);
  auto it = kvStore_map.find(req->key);
  if (it != kvStore_map.end()) {
    res->value = it->second;
    return true;
  }
  return false;
}

bool SimpleKvStore::Put(const PutRequest* req, PutResponse*) {
  // TODO (Part A, Steps 1 and 2): IMPLEMENT
  std::unique_lock<std::mutex> lock(kvStore_mutex);
  kvStore_map[req->key] = req->value;
  return true;
}

bool SimpleKvStore::Append(const AppendRequest* req, AppendResponse*) {
  // TODO (Part A, Steps 1 and 2): IMPLEMENT
  std::unique_lock<std::mutex> lock(kvStore_mutex);
  auto it = kvStore_map.find(req->key);
  if (it != kvStore_map.end()) {
    it->second.append(req->value);
    return true;
  } else {
    kvStore_map[req->key] = req->value;
    return true;
  }
  
}

bool SimpleKvStore::Delete(const DeleteRequest* req, DeleteResponse* res) {
  // TODO (Part A, Steps 1 and 2): IMPLEMENT
  std::unique_lock<std::mutex> lock(kvStore_mutex);
  auto it = kvStore_map.find(req->key);
  if (it != kvStore_map.end()) {
    res->value = it->second;
    kvStore_map.erase(it);
    return true;
  }
  return false;
}


bool SimpleKvStore::MultiGet(const MultiGetRequest* req,
                             MultiGetResponse* res) {
  // TODO (Part A, Steps 1 and 2): IMPLEMENT
  std::unique_lock<std::mutex> lock(kvStore_mutex);
  for (const auto& key : req->keys) {
    auto it = kvStore_map.find(key);
    if (it != kvStore_map.end()) {
      res->values.emplace_back(it->second);
    } else {
      return false;
    }
  }
  return true;
}

bool SimpleKvStore::MultiPut(const MultiPutRequest* req, MultiPutResponse*) {
  // TODO (Part A, Steps 1 and 2): IMPLEMENT
  std::unique_lock<std::mutex> lock(kvStore_mutex);
  if (req->keys.size() != req->values.size()) {
    return false;
  }
  for (size_t i = 0; i < req->keys.size(); ++i) {
    kvStore_map[req->keys[i]] = req->values[i];
  }
  return true;
}


std::vector<std::string> SimpleKvStore::AllKeys() {
  // TODO (Part A, Steps 1 and 2): IMPLEMENT
  std::unique_lock<std::mutex> lock(kvStore_mutex);
  std::vector<std::string> keys;
  for (const auto& kv : kvStore_map) {
    keys.emplace_back(kv.first);
  }
  return keys;
}
