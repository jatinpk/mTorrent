# mTorrent
A bit torrent like file sharing system with tracker system provides synchronization and parallel downloading. Implemented own RPC mechanism, message encoding and methods for message serialization and de-serialization. Used openssl library for computing hash values of files(SHA1)

# Architecture

![archi(1)](https://user-images.githubusercontent.com/18416045/173321968-206d914c-fefb-489c-81d4-f11c40c8aad6.png)

# Parallel Download algorithm:

* Create a 2-D map of chunk index - list of peers having that chunk of the file
  * For each chunk index i
    * Generate a random number x of range (0, len(peer_list_of_chunk))
    * Download chunk i from the peer present at index x
