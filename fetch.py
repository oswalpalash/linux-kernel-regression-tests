#!/usr/bin/env python3

import os
import requests
import re
import multiprocessing
import time
import logging
from bs4 import BeautifulSoup

# Configure basic logging
logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(levelname)s - %(message)s')

def rate_limited_get(url, last_request_time, lock):
    """
    Performs a rate-limited HTTP GET request to the specified URL.

    :param url: URL to fetch.
    :param last_request_time: Shared value indicating the last request time.
    :param lock: Lock to synchronize access to last_request_time.
    :return: Response object or None in case of an error.
    """
    try:
        with lock:
            current_time = time.time()
            if current_time - last_request_time.value < 1:
                time.sleep(1 - (current_time - last_request_time.value))
            last_request_time.value = time.time()
        response = requests.get(url)
        response.raise_for_status()
        return response
    except requests.RequestException as e:
        logging.error(f"Error fetching URL {url}: {e}")
        return None

def get_reproducers(bug, last_request_time, lock, output_dir="crepros"):
    """
    Fetches and saves reproducers for a given bug.

    :param bug: Bug URL fragment.
    :param last_request_time: Shared value indicating the last request time.
    :param lock: Lock to synchronize access to last_request_time.
    :param output_dir: Directory to save reproducers.
    """
    bug_id = bug.split("=")[1]
    existing_files = [f for f in os.listdir(output_dir) if f.startswith(bug_id)]
    if existing_files:
        logging.info(f"Files for bug {bug_id} already exist. Skipping...")
        return

    response = rate_limited_get("https://syzkaller.appspot.com" + bug, last_request_time, lock)
    if response:
        try:
            soup = BeautifulSoup(response.content, 'html.parser')
            table = soup.find_all('table', class_="list_table")[-1]
            td = table.find_all('td', string="C")
            for entry in td:
                link = entry.find('a').get('href')
                x = re.search(r'x=(.*)', link).group(1)
                repro_page = rate_limited_get("https://syzkaller.appspot.com" + link, last_request_time, lock)
                if repro_page:
                    with open(os.path.join(output_dir, f"{bug_id}-{x}.c"), 'w+') as f:
                        f.write(repro_page.text)
                    logging.info(f"Saved bug {bug_id} with x {x}")
        except Exception as e:
            logging.error(f"Error processing bug {bug_id}: {e}")

def main():
    """
    Main function to fetch and process bugs.
    """
    try:
        bugs = []
        manager = multiprocessing.Manager()
        last_request_time = manager.Value('d', time.time() - 1)
        lock = manager.Lock()

        response = rate_limited_get("https://syzkaller.appspot.com/upstream/fixed", last_request_time, lock)
        if response:
            soup = BeautifulSoup(response.content, 'html.parser')
            rows = soup.find_all('tr')
            for row in rows:
                title = row.find_all('td', class_="title")
                stat = row.find_all('td', class_="stat")
                if title and stat and "C" in stat[0].text:
                    bugs.append(title[0].find('a').get('href'))

        pool = multiprocessing.Pool(15)
        for bug in bugs:
            pool.apply_async(get_reproducers, args=(bug, last_request_time, lock))

        pool.close()
        pool.join()
    except Exception as e:
        logging.error(f"Error in main function: {e}")

if __name__ == "__main__":
    main()
