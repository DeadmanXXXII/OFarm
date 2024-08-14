use reqwest::{Client, Proxy};
use regex::Regex;
use std::sync::{Arc, Mutex};
use tokio::time::{sleep, Duration};
use std::collections::VecDeque;
use std::thread;

const TOR_PROXY: &str = "socks5://127.0.0.1:9050";
const MAX_THREADS: usize = 10;

#[derive(Clone)]
struct UrlQueue {
    queue: Arc<Mutex<VecDeque<String>>>,
}

impl UrlQueue {
    fn new() -> Self {
        UrlQueue {
            queue: Arc::new(Mutex::new(VecDeque::new())),
        }
    }

    fn enqueue(&self, url: String) {
        let mut queue = self.queue.lock().unwrap();
        queue.push_back(url);
    }

    fn dequeue(&self) -> Option<String> {                                                                                     let mut queue = self.queue.lock().unwrap();
        queue.pop_front()                                                                                                 }
}                                                                                                                     
async fn fetch_page(client: &Client, url: &str) -> Result<String, reqwest::Error> {                                       let response = client
        .get(url)                                                                                                             .timeout(Duration::from_secs(15))
        .send()
        .await?
        .text()                                                                                                               .await?;

    Ok(response)
}
                                                                                                                      fn extract_and_queue_onion_urls(page_content: &str, url_queue: &UrlQueue) {                                               let regex = Regex::new(r"([a-zA-Z2-7]{16}\.onion)").unwrap();                                                                                                                                                                               for capture in regex.captures_iter(page_content) {                                                                        let onion_url = format!("{}.onion", &capture[1]);                                                                     println!("Found .onion URL: {}", onion_url);                                                                          url_queue.enqueue(onion_url);
    }                                                                                                                 }                                                                                                                                                                                                                                           async fn worker(url_queue: UrlQueue, client: Client) {
    loop {
        match url_queue.dequeue() {
            Some(url) => {
                match fetch_page(&client, &url).await {
                    Ok(page_content) => extract_and_queue_onion_urls(&page_content, &url_queue),
                    Err(e) => eprintln!("Failed to fetch page {}: {}", url, e),
                }
            }                                                                                                                     None => {
                sleep(Duration::from_secs(1)).await;                                                                              }
        }                                                                                                                 }
}

#[tokio::main]
async fn main() -> Result<(), reqwest::Error> {
    let url_queue = UrlQueue::new();

    // Seed URLs
    let seed_urls = [
        "https://msydqstlz2kzerdg.onion",                                                                                     "https://zqktlwi4fecvo6ri.onion",                                                                                     "https://deepweblinks.onion",
        "https://onionlinksv3bdm5.onion",
        "https://hss3uro2hsxfogfq.onion",
    ];

    for &url in &seed_urls {
        url_queue.enqueue(url.to_string());
    }

    let client = Client::builder()
        .proxy(Proxy::all(TOR_PROXY)?)
        .build()?;

    let mut handles = vec![];

    for _ in 0..MAX_THREADS {
        let url_queue_clone = url_queue.clone();
        let client_clone = client.clone();

        let handle = thread::spawn(move || {
            tokio::runtime::Runtime::new().unwrap().block_on(worker(url_queue_clone, client_clone));
        });

        handles.push(handle);
    }

    for handle in handles {
        handle.join().unwrap();
    }

    Ok(())
                                   }
