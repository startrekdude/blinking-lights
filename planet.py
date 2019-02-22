import re

from contextlib import closing
from urllib2 import urlopen, Request

def main():
	url = "https://www.screensaversplanet.com/screensavers/blinking-lights-1229/"
	headers = {"User-Agent": "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/54.0.2840.99 Safari/537.36"}
	pattern = re.compile(r"UserDownloads:(\d+)")
	with closing(urlopen(Request(url, headers=headers))) as planet:
		data = planet.read()
	print int(pattern.search(data).group(1))

if __name__ == "__main__":
	main()