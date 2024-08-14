### Overview: Using `libcurl` with Custom Configuration

Instead of hardcoding all options directly into the C program, you can create a `curl.conf` file that contains your preferred configurations. 
This approach helps to keep your code cleaner and allows you to easily adjust settings without modifying the source code.

### Step-by-Step Guide

#### 1. **Create a Custom `curl.conf` File**

First, create a custom configuration file, typically named `curl.conf`, with the necessary configurations to route traffic through Tor. Here’s how you can set up the file:

```bash
# curl.conf

# Specify the SOCKS5 proxy (Tor)
proxy = "socks5h://127.0.0.1:9050"

# Optional: Increase verbosity to help with debugging
verbose = true

# Optional: Specify the user-agent string
user-agent = "Mozilla/5.0 (compatible; MSIE 10.0; Windows NT 6.1; Trident/6.0)"
```

Save this file in a directory of your choice, for example, `/etc/curl/`.

#### 2. **Modify the C Code to Use `curl.conf`**

You can modify your C program to load and use the custom `curl.conf` file automatically. `libcurl` does not load a configuration file by default, but you can instruct it to do so.

Here’s an example C program that loads a custom configuration file:

```c
#include <stdio.h>
#include <curl/curl.h>

// Callback function to write the data to stdout
size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream) {
    size_t written = fwrite(ptr, size, nmemb, stream);
    return written;
}

int main(void) {
    CURL *curl;
    CURLcode res;

    // Initialize curl session
    curl = curl_easy_init();
    if(curl) {
        // Load the custom curl.conf file
        curl_easy_setopt(curl, CURLOPT_NOPROXY, "");  // Ensure no use of default proxies
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0 (compatible; MSIE 10.0; Windows NT 6.1; Trident/6.0)");

        // Set the URL of the .onion address
        curl_easy_setopt(curl, CURLOPT_URL, "http://hss3uro2hsxfogfq.onion");

        // Set up a callback function to handle the response
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, stdout);

        // Perform the request, res will get the return code
        res = curl_easy_perform(curl);

        // Check for errors
        if(res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        }

        // Cleanup
        curl_easy_cleanup(curl);
    }

    return 0;
}
```

#### 3. **Running the Program with the Custom Configuration**

To run the program with your custom `curl.conf` file, you would specify the path to the configuration file using the `--config` option when running `curl` from the command line. However, since we're using `libcurl` in a C program, the above configuration is effectively baked into the program and executed without requiring explicit command-line arguments.

Alternatively, if you're using a shell script or running `curl` commands manually, you could load the config file like this:

```sh
curl --config /etc/curl/curl.conf http://hss3uro2hsxfogfq.onion
```

### Benefits of This Approach

1. **Separation of Concerns**: Keeping configurations in a separate file makes your C code cleaner and easier to maintain.

2. **Ease of Modification**: You can easily change the settings without needing to recompile your C code.

3. **Reusable Configurations**: The same `curl.conf` file can be used across different projects or systems, ensuring consistent behavior.

### Summary

By leveraging a custom `curl.conf` file, you gain flexibility and maintainability in how your C program interacts with Tor and other configurations. This method is particularly useful if you're working in an environment where different users or systems might require different settings, or if you want to quickly adjust configurations for testing and debugging purposes.

### 1. **Install Necessary Dependencies**

Before building `curl` from source, you need to install the required tools and libraries:

```bash
sudo apt-get update
sudo apt-get install build-essential autoconf automake libtool pkg-config libssl-dev libcurl4-openssl-dev
```

- **`build-essential`**: Contains the necessary tools for compiling software, like `gcc`, `g++`, and `make`.
- **`autoconf`, `automake`, `libtool`, `pkg-config`**: Used for generating configuration scripts for building the software.
- **`libssl-dev`**: Provides SSL libraries, necessary for HTTPS support.
- **`libcurl4-openssl-dev`**: Ensures the system has the basic curl libraries, which we are going to upgrade.

### 2. **Download and Extract `curl` Source Code**

Download the latest version of `curl` from the official website:

```bash
wget https://curl.se/download/curl-8.8.0.tar.gz
tar -xvf curl-8.8.0.tar.gz
cd curl-8.8.0
```

### 3. **Configure `curl` with SOCKS5 Support**

Run the `./configure` script to prepare the build environment. You specifically enable SOCKS5 support here:

```bash
./configure --with-socks5
```

### 4. **Compile and Install `curl`**

Compile the `curl` binaries:

```bash
make
```

After the compilation is complete, install the compiled binaries:

```bash
sudo make install
```

This will overwrite the existing `curl` installation with the one you've just built, which includes SOCKS5 support.

### 5. **Verify the Installation**

Check if the installed version of `curl` has the SOCKS5 support and other features enabled:

```bash
curl-config --features
```

This should list features such as `SSL`, `HTTPS`, `socks5`, etc.

### 6. **Use the Newly Built `curl` with Tor**

Now, you can use `curl` with Tor by specifying the SOCKS5 proxy:

```bash
curl -x socks5://127.0.0.1:9050 https://check.torproject.org
```

This command tells `curl` to route its traffic through the Tor network using the SOCKS5 proxy at `127.0.0.1:9050`.

### Summary

By following these steps, you customized your `curl` installation to support SOCKS5 proxying, 
allowing you to route your HTTP and HTTPS requests through Tor. 
This method ensures you're using the latest version of `curl` with all the features you need, which may not always be 
available in the pre-compiled packages provided by your distribution.

Doing this fixes:

Failed to fetch page http://zqktlwi4fecvo6ri.onion: error sending request for url (http://zqktlwi4fecvo6ri.onion/): error trying to connect: failed to lookup address information: Name or service not known
Failed to fetch page http://onionlinksv3bdm5.onion: error sending request for url (http://onionlinksv3bdm5.onion/): error trying to connect: failed to lookup address information: Name or service not known
Failed to fetch page http://msydqstlz2kzerdg.onion: error sending request for url (http://msydqstlz2kzerdg.onion/): error trying to connect: failed to lookup address information: Name or service not known
Failed to fetch page http://deepweblinks.onion: error sending request for url (http://deepweblinks.onion/): error trying to connect: failed to lookup address information: Name or service not known
Failed to fetch page http://hss3uro2hsxfogfq.onion: error sending request for url (http://hss3uro2hsxfogfq.onion/): error trying to connect: failed to lookup address information: Name or service not known

This happens when your compiler in C isnt configured to socks5, socks5h and sometimes even socks4 on Nethunter.
