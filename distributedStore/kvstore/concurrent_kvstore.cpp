#include "concurrent_kvstore.hpp"

#include <optional>
#include <set>

bool ConcurrentKvStore::Get(const GetRequest* req, GetResponse* res) {
  // TODO (Part A, Steps 4 and 5): IMPLEMENT
  size_t bucket_idx = store.bucket(req->key);
  std::shared_lock<std::shared_mutex> guard(store.bucket_locks[bucket_idx]);
  std::optional<DbItem> db_res = store.getIfExists(bucket_idx, req->key);
  if (db_res == std::nullopt) {
    return false;
  }
  res->value = db_res->value;
  return true;
}

bool ConcurrentKvStore::Put(const PutRequest* req, PutResponse*) {
  // TODO (Part A, Steps 4 and 5): IMPLEMENT
  size_t bucket_idx = store.bucket(req->key);
  std::unique_lock<std::shared_mutex> guard(store.bucket_locks[bucket_idx]);
  store.insertItem(bucket_idx, req->key, req->value);
  return true;
}

bool ConcurrentKvStore::Append(const AppendRequest* req, AppendResponse*) {
  // TODO (Part A, Steps 4 and 5): IMPLEMENT
  size_t bucket_idx = store.bucket(req->key);
  std::unique_lock<std::shared_mutex> guard(store.bucket_locks[bucket_idx]);
  std::optional<DbItem> db_res = store.getIfExists(bucket_idx, req->key);
  if (db_res != std::nullopt) {
    db_res->value += req->value;
    store.insertItem(bucket_idx, req->key, db_res->value);
  } else {
    store.insertItem(bucket_idx, req->key, req->value);
  }
  return true;
}

bool ConcurrentKvStore::Delete(const DeleteRequest* req, DeleteResponse* res) {
  // TODO (Part A, Steps 4 and 5): IMPLEMENT
  size_t bucket_idx = store.bucket(req->key);
  std::unique_lock<std::shared_mutex> guard(store.bucket_locks[bucket_idx]);
  std::optional<DbItem> db_res = store.getIfExists(bucket_idx, req->key);
  if (db_res != std::nullopt) {
    res->value = db_res->value;
    store.removeItem(bucket_idx, req->key);
    return true;
  }
  return false;
}

bool ConcurrentKvStore::MultiGet(const MultiGetRequest* req,
                                 MultiGetResponse* res) {
  // TODO (Part A, Steps 4 and 5): IMPLEMENT
  //unique sorted set
  std::set<size_t> ordered_buckets;
  for (unsigned int i = 0; i < req->keys.size(); i++) {
    ordered_buckets.insert(store.bucket(req->keys[i]));
  }
  // locking all
  std::set<size_t>::iterator itr;
  for (itr = ordered_buckets.begin(); itr != ordered_buckets.end(); itr++) {
    store.bucket_locks[*itr].lock_shared();
  }
  // multiget implementation
  for (const auto& key : req->keys) {
    size_t bucket_idx = store.bucket(key);
    std::optional<DbItem> db_res = store.getIfExists(bucket_idx, key);
    if (db_res != std::nullopt) {
      res->values.emplace_back(db_res->value);
    } else {
      return false;
    }
  }
  // unlocking all
  for (itr = ordered_buckets.begin(); itr != ordered_buckets.end(); itr++) {
    store.bucket_locks[*itr].unlock_shared();
  }
  return true;
}

bool ConcurrentKvStore::MultiPut(const MultiPutRequest* req,
                                 MultiPutResponse*) {
  // TODO (Part A, Steps 4 and 5): IMPLEMENT
  // unique ordered sorted set
  std::set<size_t> ordered_buckets;
  for (unsigned int i = 0; i < req->keys.size(); i++) {
    ordered_buckets.insert(store.bucket(req->keys[i]));
  }
  // locking all
  std::set<size_t>::iterator itr;
  for (itr = ordered_buckets.begin(); itr != ordered_buckets.end(); itr++) {
    store.bucket_locks[*itr].lock();
  }
  // multiput implementation
  if (req->keys.size() != req->values.size()) {
    return false;
  }
  for (size_t i = 0; i < req->keys.size(); i++) {
    size_t bucket_idx = store.bucket(req->keys[i]);
    store.insertItem(bucket_idx, req->keys[i], req->values[i]);
  }
  // unlocking all
  for (itr = ordered_buckets.begin(); itr != ordered_buckets.end(); itr++) {
    store.bucket_locks[*itr].unlock();
  }
  return true;
}

std::vector<std::string> ConcurrentKvStore::AllKeys() {
  // TODO (Part A, Steps 4 and 5): IMPLEMENT
  // unique ordered sorted set
  std::set<size_t> ordered_buckets;
  for (auto db_item_list: store.buckets) {
    for (auto db_item: db_item_list) {
      ordered_buckets.insert(store.bucket(db_item.key));
    }
  }
  // locking all
  std::set<size_t>::iterator itr;
  for (itr = ordered_buckets.begin(); itr != ordered_buckets.end(); itr++) {
    store.bucket_locks[*itr].lock_shared();
  }
  // all keys implementation
  std::vector<std::string> keys;
  for (size_t i = 0; i < store.BUCKET_COUNT; ++i) {
    //std::shared_lock<std::shared_mutex> lock(store.bucket_locks[i]);
    for (const auto& item : store.buckets[i]) {
      keys.emplace_back(item.key);
    }
  }
  // unlocking all
  for (itr = ordered_buckets.begin(); itr != ordered_buckets.end(); itr++) {
    store.bucket_locks[*itr].unlock_shared();
  }
  return keys;
}
