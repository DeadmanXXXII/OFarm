# OFarm
A .onion website finder and verifier.
You must have tor configured correctly you can use the command below and you should get the same output.

![Tor confirmed Nethunter](https://raw.githubusercontent.com/DeadmanXXXII/OFarm/main/Screenshot_20240814-233454.png)


### Full Implementation with Automatic Seed URL Discovery

### Explanation of the Code

1. **Initial Seed URL Queue**:
    - The `queue_initial_urls` function queues the initial URLs that the program will begin crawling from. 
    - These URLs should ideally be directories or known sources of `.onion` URLs.
    
2. **URL Extraction and Queueing**:
    - `extract_and_queue_onion_urls` is responsible for extracting `.onion` URLs from the fetched page content and queuing them for further processing.

3. **Thread Function**:
    - `thread_func` constantly pulls URLs from the queue and processes them to find more `.onion` URLs. It keeps running until the queue is empty.

4. **Multi-threading**:
    - The program creates multiple threads (`MAX_THREADS` is set to 10) to process URLs concurrently. This significantly improves the speed of finding `.onion` URLs.

5. **Recursive Crawling**:
    - Each thread fetches a URL, extracts any `.onion` URLs from it, and queues them for further processing. This way, the program recursively discovers new `.onion` URLs.

### Step-by-Step Guide to Running the Program

1. **Install Necessary Tools**:

    Make sure to have the required tools installed:

    ```bash
    sudo apt-get update
    sudo apt-get install gcc libcurl4-openssl-dev tor
    ```

2. **Start Tor**:

    Ensure the Tor service is running:

    ```bash
    sudo service tor start
    ```

3. **Create the Program File**:

    Create a file named `OFarm.c` and paste the provided code:

    ```bash
    nano OFarm.c
    ```

    - Paste the code into this file.
    - Save the file with `Ctrl + O` and exit with `Ctrl + X`.

4. **Compile the Program**:

    Compile the program using `gcc`:

    ```bash
    gcc -o OFarm OFarm.c -lcurl -lpthread
    ```

5. **Run the Program**:

    Execute the compiled program:

    ```bash
    ./OFarm
    ```

6. **Monitor the Output**:

    The program will start by processing the seed URLs and will continue to find and queue new `.onion` URLs. The output will display any discovered `.onion` URLs along with their source.

### Extending the Program

- **Adding Real Seed URLs**: You should replace the placeholder URLs in the `seed_urls[]` array with actual `.onion` directory URLs or other sources of `.onion` links.
  
- **Handling Dead URLs**: You could enhance the error handling to manage dead URLs, possibly removing them from the queue or logging them for later analysis.

### Conclusion

This guide provides a detailed implementation and step-by-step instructions to create a self-sufficient `.onion` finder that starts with its own seed URLs. By continuously crawling and discovering new `.onion` sites, the program can autonomously gather and verify `.onion` URLs in the background.
