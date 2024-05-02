import json
import requests

proxy_servers = {
    'http': "http://s_lrcfnc06e03f240n:pippococa:@10.0.0.1:800",
    # 'https': "https://s_lrcfnc06e03f240n:pippococa@10.0.0.1:800"
}

temperature_server = "http://10.25.0.14:3000"
remote_post_url = "http://portale.itisurbino.it:8080/~s_glnglc06e28l500q/postListener"
local_post_url = "http://192.168.1.3:10000/postListener"
at_school_post_url = "http://10.25.0.15:10000/postListener"

def get_data():
    # Use in school: response = requests.get(request_server, proxies=proxy_servers)
    response = requests.get(temperature_server)

    if response.status_code == 200:
        data = response.text
        print(data)
    else:
        print("An error occured: %d", response.status_code)


def make_post_request():
    test_data = {"device_id": "pc_test", "temperature": [18, 19, 21], "humidity": [50, 62, 31], "heat_index": [2, 5, 8], "light": [200, 400, 532]}
    print(local_post_url)
    x = requests.post(local_post_url, json=test_data)

    print(x.text)


if __name__ == "__main__":
    make_post_request()
