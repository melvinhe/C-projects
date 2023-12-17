#include "simple_client.hpp"

std::optional<std::string> SimpleClient::Get(const std::string& key) {
  std::shared_ptr<ServerConn> conn = connect_to_server(this->server_addr);
  if (!conn) {
    cerr_color(RED, "Failed to connect to KvServer at ", this->server_addr,
               '.');
    return std::nullopt;
  }

  GetRequest req{key};
  if (!conn->send_request(req)) return std::nullopt;

  std::optional<Response> res = conn->recv_response();
  if (!res) return std::nullopt;
  if (auto* get_res = std::get_if<GetResponse>(&*res)) {
    return get_res->value;
  } else if (auto* error_res = std::get_if<ErrorResponse>(&*res)) {
    cerr_color(RED, "Failed to Get value from server: ", error_res->msg);
  }

  return std::nullopt;
}

bool SimpleClient::Put(const std::string& key, const std::string& value) {
  std::shared_ptr<ServerConn> conn = connect_to_server(this->server_addr);
  if (!conn) {
    cerr_color(RED, "Failed to connect to KvServer at ", this->server_addr,
               '.');
    return false;
  }

  PutRequest req{key, value};
  if (!conn->send_request(req)) return false;

  std::optional<Response> res = conn->recv_response();
  if (!res) return false;
  if (auto* put_res = std::get_if<PutResponse>(&*res)) {
    return true;
  } else if (auto* error_res = std::get_if<ErrorResponse>(&*res)) {
    cerr_color(RED, "Failed to Put value to server: ", error_res->msg);
  }

  return false;
}

bool SimpleClient::Append(const std::string& key, const std::string& value) {
  std::shared_ptr<ServerConn> conn = connect_to_server(this->server_addr);
  if (!conn) {
    cerr_color(RED, "Failed to connect to KvServer at ", this->server_addr,
               '.');
    return false;
  }

  AppendRequest req{key, value};
  if (!conn->send_request(req)) return false;

  std::optional<Response> res = conn->recv_response();
  if (!res) return false;
  if (auto* append_res = std::get_if<AppendResponse>(&*res)) {
    return true;
  } else if (auto* error_res = std::get_if<ErrorResponse>(&*res)) {
    cerr_color(RED, "Failed to Append value to server: ", error_res->msg);
  }

  return false;
}

std::optional<std::string> SimpleClient::Delete(const std::string& key) {
  std::shared_ptr<ServerConn> conn = connect_to_server(this->server_addr);
  if (!conn) {
    cerr_color(RED, "Failed to connect to KvServer at ", this->server_addr,
               '.');
    return std::nullopt;
  }

  DeleteRequest req{key};
  if (!conn->send_request(req)) return std::nullopt;

  std::optional<Response> res = conn->recv_response();
  if (!res) return std::nullopt;
  if (auto* delete_res = std::get_if<DeleteResponse>(&*res)) {
    return delete_res->value;
  } else if (auto* error_res = std::get_if<ErrorResponse>(&*res)) {
    cerr_color(RED, "Failed to Delete value on server: ", error_res->msg);
  }

  return std::nullopt;
}

std::optional<std::vector<std::string>> SimpleClient::MultiGet(
    const std::vector<std::string>& keys) {
  std::shared_ptr<ServerConn> conn = connect_to_server(this->server_addr);
  if (!conn) {
    cerr_color(RED, "Failed to connect to KvServer at ", this->server_addr,
               '.');
    return std::nullopt;
  }

  MultiGetRequest req{keys};
  if (!conn->send_request(req)) return std::nullopt;

  std::optional<Response> res = conn->recv_response();
  if (!res) return std::nullopt;
  if (auto* multiget_res = std::get_if<MultiGetResponse>(&*res)) {
    return multiget_res->values;
  } else if (auto* error_res = std::get_if<ErrorResponse>(&*res)) {
    cerr_color(RED, "Failed to MultiGet values on server: ", error_res->msg);
  }

  return std::nullopt;
}

bool SimpleClient::MultiPut(const std::vector<std::string>& keys,
                            const std::vector<std::string>& values) {
  std::shared_ptr<ServerConn> conn = connect_to_server(this->server_addr);
  if (!conn) {
    cerr_color(RED, "Failed to connect to KvServer at ", this->server_addr,
               '.');
    return false;
  }

  MultiPutRequest req{keys, values};
  if (!conn->send_request(req)) return false;

  std::optional<Response> res = conn->recv_response();
  if (!res) return false;
  if (auto* multiput_res = std::get_if<MultiPutResponse>(&*res)) {
    return true;
  } else if (auto* error_res = std::get_if<ErrorResponse>(&*res)) {
    cerr_color(RED, "Failed to MultiPut values on server: ", error_res->msg);
  }

  return false;
}

std::vector<std::string> split_keys(const std::string posts) {
  // split_keys helps split a comma separated string into individual strings in a vector
  std::vector<std::string> post_keys;
  // Trim trailing whitespace from the input string
  std::string trimmed_posts = posts.substr(0, posts.find_last_not_of(" \n\r\t") + 1);
  std::stringstream ss(trimmed_posts);
  std::string post_key;
  while (std::getline(ss, post_key, ',')) {
    post_keys.push_back(post_key);
  }
  return post_keys;
}


std::optional<std::string> join_keys(std::vector<std::string> keys) {
  // join_keys is somewhat opposite of split_keys, joining together strings in a vector 
  if (keys.empty()) {
    return std::nullopt;
  }
  std::stringstream ss;
  for (auto it = keys.begin(); it != keys.end() - 1; ++it) {
    ss << *it << ",";
  }
  ss << keys.back();
  return ss.str();
}

bool SimpleClient::GDPRDelete(const std::string& user) {
  // TODO: Write your GDPR deletion code here!
  // You can invoke operations directly on the client object, like so:
  //
  // std::string key("user_1_posts");
  // std::optional<std::string> posts = Get(key);
  // ...
  //
  // Assume the `user` arugment is a user ID such as "user_1".

  /* The folloing GDPRDelete implementation deletes data in database corresponding to the
   * user's profile account, user's account reference in all_users, all of user's posts,
   * and list of individual references to all of user's posts.
   * 
   * The implementation opts out of deleting replies to the user's posts from other users and
   * references to the user from other users posts.
   * 
   * I also left the BLUE "cerr_color" print lines since that I felt that they were useful when
   * debugging and looked kind of neat. If you don't find them necessary, you may remove or comment
   * them out.
   */

  std::string profile_key(user); // key corresponding to user (ex: user_1)
  std::string all_posts_key(user + "_posts"); // key corresponding to user_posts (ex: user_1_posts)
  std::optional<std::string> user_posts = Get(all_posts_key);

  // Deleting all of user's individual posts
  if (user_posts) {
    std::vector<std::string> post_keys = split_keys(user_posts.value());
    for (const auto& post_key : post_keys) {
      cerr_color(BLUE, "Deleting post key: " + post_key + "!");
      std::optional<std::string> get_post = Get(post_key);
      if (get_post) {
        Delete(post_key);
      }
    }
  }
  
  // Deleting data consisting of a list with all of user's posts
  cerr_color(BLUE, "Deleting post key: " + all_posts_key + "!");
  Delete(all_posts_key);

  // Removing references to user's personal profile in all_users list 
  std::optional<std::string> all_users = Get("all_users");
  if (all_users) {
    std::vector<std::string> all_users_keys = split_keys(all_users.value());
    cerr_color(BLUE, "Removing user profile from all_users: " + profile_key + "!");
    std::vector<std::string>::iterator it = std::find(all_users_keys.begin(), all_users_keys.end(), profile_key);
    if (it != all_users_keys.end()) {
      all_users_keys.erase(it);
    }
    std::optional<std::string> joint_keys = join_keys(all_users_keys);
    if (joint_keys) {
      Put("all_users", join_keys(all_users_keys).value());
    }
  }

  // Deleting user's personal profile
  cerr_color(BLUE, "Deleting post key: " + profile_key + "!");
  Delete(profile_key);
  
  return true;
}
