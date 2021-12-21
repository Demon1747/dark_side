// Copyright 2021 Demon1747 Specdok117@mail.ru

#include "ui.h"

#include <boost/filesystem/operations.hpp>
#include <fstream>
#include <iostream>

#include "database.h"
#include "encrypt.h"
#include "convert.h"

bool database_redactor(const std::string& db_path) {
  std::cout << "Welcome to the database redactor!" << std::endl;

  std::string in_uname;
  std::string in_pw;

  std::cout << "Enter username: ";
  std::cin >> in_uname;
  std::cout << "Enter password: ";
  std::cin >> in_pw;

  std::string really_strong_password(
      "D82494F05D6917BA02F7AAA29689CCB444BB73F20380876CB05D1F37537B7892");

  if (in_uname != "admin" ||
      Botan::hex_encode(hash(str_to_bytes(in_pw))) != really_strong_password) {
    std::cout << "Access denied!" << std::endl;
    exit(EXIT_FAILURE);
  }

  std::cout << std::endl << "Access granted" << std::endl << std::endl;

  std::fstream db_file("users.db");
  if (!db_file.is_open()) {
    Database db;
  }

  Database db(db_path);

  boost::system::error_code err;

  if (!boost::filesystem::exists("public_keys")) {
    boost::filesystem::create_directory("public_keys", err);
  }
  if (err) {
    std::cout << "Failed to create a directory" << std::endl;
  }

  std::cout << std::endl;
  while (true) {
    std::cout << "Available commands:" << std::endl
              << "  p [ print-users ] " << std::endl
              << "  a [ add-user ] " << std::endl
              << "  d [ delete-user ] " << std::endl
              << "  e [ exit ] " << std::endl
              << std::endl
              << "Print your option here: ";

    std::string command;

    std::cin >> command;
    std::cout << std::endl;
    if (command == "p" || command == "print-users") {
      db.print_users();
    } else if (command == "a" || command == "add-user") {
      add_user(db);
    } else if (command == "d" || command == "delete-user") {
      delete_user(db);
    } else if (command == "e" || command == "exit") {
      exit(EXIT_SUCCESS);
    } else {
      std::cout << "Invalid command, terminating..." << std::endl;
      exit(EXIT_FAILURE);
    }

    std::string line(80, '-');
    std::cout << line << std::endl << std::endl;
  }
}

void add_user(Database& db) {  //}, const std::string& status) {
  /// Проверяется статус редактора базы данных
  //  if (status != "admin") {
  //    std::cerr << "Access denied" << std::endl;
  //    return;
  //  }

  ///
  size_t in_id;
  std::string in_status;
  std::string in_username;
  std::string in_address;
  std::string in_password;
  size_t in_port;
  std::string priv_key_password;

  /// Entering data
  std::cout << "Enter user ID: ";
  std::cin >> in_id;
  std::cout << "Enter user status: ";
  std::cin >> in_status;
  std::cout << "Enter user name: ";
  std::cin >> in_username;
  std::cout << "Enter user password: ";
  std::cin >> in_password;
  std::cout << "Enter user address: ";
  std::cin >> in_address;
  std::cout << "Enter user port: ";
  std::cin >> in_port;
  std::cout << "Set user private key password (don't forget it!): ";
  std::cin >> priv_key_password;

  /// Hashing user password
  std::vector<uint8_t> hash_in_bytes =
      hash(str_to_bytes(in_password + std::to_string(in_id)));
  in_password = Botan::hex_encode(hash_in_bytes);

  std::string pub_key_storage("public_keys/public_k_" + std::to_string(in_id) +
                              ".txt");
  std::string priv_key_storage("private_" + std::to_string(in_id));

  std::unique_ptr<Botan::RandomNumberGenerator> rng(new Botan::System_RNG);
  //  rng.reset(new Botan::System_RNG);

  /// Generating key pair
  size_t pub_key_size = 4096;

  Botan::RSA_PrivateKey key_pair(*rng, pub_key_size);
  std::string public_key = Botan::X509::PEM_encode(key_pair);
  std::string priv_key(
      Botan::PKCS8::PEM_encode(key_pair, *rng, priv_key_password));

  std::ofstream pub_key_file(pub_key_storage, std::ofstream::binary);
  pub_key_file.write(public_key.data(),
                     static_cast<std::streamsize>(public_key.size()));
  pub_key_file.close();

  std::ofstream priv_key_file(priv_key_storage, std::ofstream::binary);
  priv_key_file.write(priv_key.data(),
                      static_cast<std::streamsize>(priv_key.size()));
  priv_key_file.close();

  //  std::unique_ptr<Botan::Private_Key> keeeeey(Botan::PKCS8::load_key(
  //      "private_" + std::to_string(in_id), *rng, priv_key_password));
  //
  //  std::unique_ptr<Botan::Public_Key> keeeeey2(Botan::X509::load_key(
  //      "public_keys/public_k_" + std::to_string(in_id) + ".txt"));

  User input(in_id, in_status, in_username, in_password, in_address, in_port,
             pub_key_storage);
  db.insert_user(input);

  std::cout << std::endl
            << "A new user was added successfully!" << std::endl
            << std::endl;
}

void delete_user(Database& db) {
  db.print_users();
  size_t del_id;
  std::cout << "Enter id of user you want to delete: ";
  std::cin >> del_id;
  std::cout << std::endl;

  try {
    std::string pub_key_path(db.extract_user_by_id(del_id).key_storage());
    std::string priv_key_path("private_" + std::to_string(del_id));

    std::remove(pub_key_path.data());
    std::remove(priv_key_path.data());

    db.delete_user(del_id);
  } catch (const std::invalid_argument& err) {
    std::cout << err.what() << std::endl;
    return;
  }
  std::cout << "User was deleted successfully!" << std::endl << std::endl;
}

bool log_in(Database& db) {
  std::string in_uname;
  User incoming_user;
  for (size_t i = 0; i < 3; ++i) {
    std::cout << "Enter username: ";
    std::cin >> in_uname;
    try {
      incoming_user = db.extract_user_by_un(in_uname);
    } catch (const std::invalid_argument& e) {
      std::cout << e.what() << std::endl;
      if (i == 2) {
        std::cout << "Something got wrong, see you next time!" << std::endl;
        exit(EXIT_FAILURE);
      }
      continue;
    }
    break;
  }

  std::string input_password;
  for (size_t i = 0; i < 3; ++i) {
    std::cout << "Enter your password: ";
    std::cin >> input_password;
    std::vector<uint8_t> pw_in_bytes =
        hash(str_to_bytes(input_password + std::to_string(incoming_user.id())));

    input_password = Botan::hex_encode(pw_in_bytes);
    ///Пароль в структуре User хранится в хэшированном виде
    if (incoming_user.check_pw(input_password)) {
      std::cout << "Welcome to the club, buddy!" << std::endl;
      std::string line(80, '-');
      std::cout << line << std::endl;
      return true;
    }
    if (i == 2) {
      std::cerr << "Access denied" << std::endl;
      exit(EXIT_FAILURE);
    }
    std::cout << "Incorrect password, try again!" << std::endl;
  }
}
