# trace/test_plot.py
import json

test_data = {
    "levels": [
        {
            "total_size": 65536,
            "blocks": [
                {"offset": 0, "size": 2048, "free": False},
                {"offset": 2048, "size": 4096, "free": True}
            ]
        },
        {
            "total_size": 131072,
            "blocks": [
                {"offset": 0, "size": 8192, "free": False}
            ]
        }
    ]
}

with open("trace.json", "w") as f:
    json.dump(test_data, f, indent=2)
