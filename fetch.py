#!/usr/bin/env python3

import os
import requests
import re
import multiprocessing
import time
import logging
from bs4 import BeautifulSoup

logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(levelname)s - %(message)s')

SYZ_BASE = "https://syzkaller.appspot.com"


def rate_limited_get(url, last_request_time, lock, timeout=60):
    """
    Performs a rate-limited HTTP GET request to the specified URL.
    """
    try:
        with lock:
            current_time = time.time()
            if current_time - last_request_time.value < 1:
                time.sleep(1 - (current_time - last_request_time.value))
            last_request_time.value = time.time()
        response = requests.get(url, timeout=timeout)
        response.raise_for_status()
        return response
    except requests.RequestException as e:
        logging.error("Error fetching URL %s: %s", url, e)
        return None


def has_c_repro(row):
    """
    True if this fixed-bug table row advertises a C reproducer.

    Column layout (list_table): Title | Rank | Repro | ... where Rank and Repro
    both use class="stat". Older code checked stat[0] (Rank) and always missed.
    """
    title = row.find("td", class_="title")
    if not title or not title.find("a"):
        return False
    for td in row.find_all("td", class_="stat"):
        if td.get_text(strip=True) == "C":
            return True
        # Sometimes the letter is only inside the link text
        for a in td.find_all("a"):
            if a.get_text(strip=True) == "C":
                return True
    return False


def get_reproducers(bug, last_request_time, lock, output_dir="crepros"):
    """
    Fetches and saves C reproducers for a given bug URL fragment.
    """
    bug_id = bug.split("=")[-1]
    try:
        existing = any(name.startswith(bug_id) for name in os.listdir(output_dir))
    except FileNotFoundError:
        existing = False
    if existing:
        logging.info("Files for bug %s already exist. Skipping...", bug_id)
        return

    response = rate_limited_get(SYZ_BASE + bug, last_request_time, lock)
    if not response:
        return

    try:
        soup = BeautifulSoup(response.content, "html.parser")
        tables = soup.find_all("table", class_="list_table")
        if not tables:
            logging.warning("No list_table on bug %s", bug_id)
            return

        # Prefer crash tables that actually link to C repro text
        saved = 0
        for table in tables:
            for a in table.find_all("a"):
                if a.get_text(strip=True) != "C":
                    continue
                link = a.get("href")
                if not link:
                    continue
                m = re.search(r"x=([^&]+)", link)
                if not m:
                    continue
                x = m.group(1)
                out_path = os.path.join(output_dir, f"{bug_id}-{x}.c")
                if os.path.exists(out_path):
                    continue
                repro_page = rate_limited_get(SYZ_BASE + link, last_request_time, lock)
                if not repro_page:
                    continue
                with open(out_path, "w", encoding="utf-8", errors="replace") as f:
                    f.write(repro_page.text)
                saved += 1
                logging.info("Saved bug %s with x %s", bug_id, x)
        if saved == 0:
            logging.info("No new C repro text for bug %s", bug_id)
    except Exception as e:
        logging.error("Error processing bug %s: %s", bug_id, e)


def main():
    output_dir = "crepros"
    os.makedirs(output_dir, exist_ok=True)

    manager = multiprocessing.Manager()
    last_request_time = manager.Value("d", time.time() - 1)
    lock = manager.Lock()

    response = rate_limited_get(f"{SYZ_BASE}/upstream/fixed", last_request_time, lock, timeout=120)
    if not response:
        raise SystemExit("Failed to fetch upstream/fixed bug list")

    soup = BeautifulSoup(response.content, "html.parser")
    bugs = []
    for row in soup.find_all("tr"):
        if not has_c_repro(row):
            continue
        href = row.find("td", class_="title").find("a").get("href")
        if href:
            bugs.append(href)

    # Stable unique order
    bugs = list(dict.fromkeys(bugs))
    logging.info("Found %d bugs with C reproducers", len(bugs))

    # Modest pool; rate limiter serializes HTTP to ~1 req/s
    with multiprocessing.Pool(8) as pool:
        pool.starmap(
            get_reproducers,
            ((bug, last_request_time, lock, output_dir) for bug in bugs),
        )
    logging.info("Fetch complete")


if __name__ == "__main__":
    main()
