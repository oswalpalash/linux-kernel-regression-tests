#!/usr/bin/env python3

from bs4 import BeautifulSoup
import os
import requests
import re
import multiprocessing
import time

def rate_limited_get(url, last_request_time):
    while True:
        with last_request_time.get_lock():
            current_time = time.time()
            if current_time - last_request_time.value >= 1:
                last_request_time.value = current_time
                return requests.get(url)
            else:
                time.sleep(1 - (current_time - last_request_time.value))
'''
Query https://syzkaller.appspot.com/upstream/fixed for all bugs that have been fixed and have "C" and "syz" reproducers
Save reproducers to text files
'''

def get_reproducers(bug, last_request_time):
    bug_id = bug.split("=")[1]
    existing_files = [f for f in os.listdir("crepros/") if f.startswith(bug_id)]
    if existing_files:
        print(f"Files for bug {bug_id} already exist. Skipping...")
        return

    page = rate_limited_get("https://syzkaller.appspot.com" + bug, last_request_time)
    soup = BeautifulSoup(page.content, 'html.parser')
    table = soup.find_all('table', class_="list_table")[-1]
    td = table.find_all('td', string="C")
    for entry in td:
        link = entry.find('a').get('href')
        page = rate_limited_get("https://syzkaller.appspot.com" + link, last_request_time)
        x = re.search(r'x=(.*)', link).group(1)
        print("Saving bug " + bug_id + " with x " + x)
        with open("crepros/" + bug_id + "-" + x + ".c", 'w+') as f:
            f.write(page.text)

def main():
    bugs = []
    manager = multiprocessing.Manager()
    last_request_time = manager.Value('d', time.time() - 1)

    page = rate_limited_get("https://syzkaller.appspot.com/upstream/fixed", last_request_time)
    soup = BeautifulSoup(page.content, 'html.parser')
    rows = soup.find_all('tr')
    for row in rows:
        title = row.find_all('td', class_="title")
        stat = row.find_all('td', class_="stat")
        if title and stat and "C" in stat[0].text:
            bugs.append(title[0].find('a').get('href'))

    pool = multiprocessing.Pool(15)
    for bug in bugs:
        pool.apply_async(get_reproducers, args=(bug, last_request_time))
    
    pool.close()
    pool.join()

if __name__ == "__main__":
    main()
