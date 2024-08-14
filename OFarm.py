import requests
import re
import threading
from queue import Queue
from time import sleep

TOR_PROXY = "socks5://127.0.0.1:9050"
MAX_THREADS = 10

# Queue for URLs
url_queue = Queue()

def fetch_page(url):
    try:
        response = requests.get(url, proxies={"http": TOR_PROXY, "https": TOR_PROXY}, timeout=15)
        response.raise_for_status()
        return response.text
    except requests.RequestException as e:
        print(f"Failed to fetch {url}: {e}")
        return None

def extract_and_queue_onion_urls(page_content, origin_url):
    pattern = r"([a-zA-Z2-7]{16}\.onion)"
    matches = re.findall(pattern, page_content)
    for match in matches:
        print(f"Found .onion URL: {match} (found on {origin_url})")
        url_queue.put(match)

def thread_func():
    while True:
        current_url = url_queue.get()
        if current_url is None:
            break
        page_content = fetch_page(current_url)
        if page_content:
            extract_and_queue_onion_urls(page_content, current_url)
        url_queue.task_done()

def queue_initial_urls():
    seed_urls = [
        "http://msydqstlz2kzerdg.onion",  # Ahmia
        "http://zqktlwi4fecvo6ri.onion",  # The Hidden Wiki
        "http://deepweblinks.onion",      # Deep Web Links
        "http://onionlinksv3bdm5.onion",  # OnionLinks
        "http://hss3uro2hsxfogfq.onion",  # Not Evil
    ]

    for url in seed_urls:
        url_queue.put(url)

def main():
    queue_initial_urls()

    threads = []
    for _ in range(MAX_THREADS):
        thread = threading.Thread(target=thread_func)
        thread.start()
        threads.append(thread)

    url_queue.join()

    for _ in range(MAX_THREADS):
        url_queue.put(None)
    for thread in threads:
        thread.join()

if __name__ == "__main__":
    main()
