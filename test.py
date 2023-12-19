import argparse
from client import Client
def main():
    parser = argparse.ArgumentParser(description='A simple script with argparse')

    # Add positional argument
    parser.add_argument('-u', '--uploadRequest')
    parser.add_argument('-d', '--downloadRequest')
    parser.add_argument('-l', '--fileListRequest')
    parser.add_argument('-p', '--path')
    parser.add_argument('-f', '--filename')
    parser.add_argument('-r', '--port')
    parser.add_argument('-a', '--address')
    # Parse the arguments
    args = parser.parse_args()
    client = Client(args.address, int(args.port))
    client.connect()
    if args.uploadRequest:
        client.upload(args.filename, args.path)
    elif args.downloadRequest:
        client.download(args.filename)

if __name__ == '__main__':
    main()

