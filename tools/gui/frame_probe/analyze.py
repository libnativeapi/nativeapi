"""Join compositor QPC samples with an application's paced frame records."""

import argparse
import collections
import itertools
import json
import math
import statistics
from pathlib import Path


def statistics_for(values):
    values = sorted(values)
    if not values:
        return {"count": 0}
    return {
        "count": len(values),
        "mean_ms": statistics.mean(values),
        "p95_ms": values[math.ceil(len(values) * 0.95) - 1],
        "p99_ms": values[math.ceil(len(values) * 0.99) - 1],
        "max_ms": values[-1],
    }


def analyze(capture, producer):
    records = [json.loads(line) for line in Path(capture).read_text().splitlines()]
    meta = records[0]
    if records[-1].get("type") != "complete":
        raise ValueError("Capture did not finish successfully")
    app = json.loads(Path(producer).read_text(encoding="utf-8-sig"))
    frequency = app["frequency"]
    if frequency != meta["frequency"]:
        raise ValueError("Capture and producer clocks disagree")
    samples = [s for s in app["samples"] if s["phase"] == "steady"]
    if len(samples) < 2:
        raise ValueError("Need at least two steady producer samples")
    start, end = samples[0]["start_qpc"], samples[-1]["finish_qpc"]
    observations = [r for r in records if r.get("type") == "frame"]
    if (
        not observations
        or observations[0]["present_qpc"] > start
        or observations[-1]["present_qpc"] < end
    ):
        raise ValueError("Capture does not cover the entire steady producer interval")
    frames = [
        r
        for r in records
        if r.get("type") == "frame" and start <= r["present_qpc"] <= end
    ]
    changed = [r for r in frames if r["changed"]]
    if len(changed) < 2:
        raise ValueError("Need at least two changed composed frames")
    intervals = [
        (b["present_qpc"] - a["present_qpc"]) * 1000 / frequency
        for a, b in itertools.pairwise(changed)
    ]
    hashes = collections.Counter(r["hash"] for r in changed)
    refresh_hz = (
        meta["dwm_refresh_numerator"] / meta["dwm_refresh_denominator"]
        if meta["dwm_refresh_denominator"]
        else meta["refresh_hz"]
    )
    result = {
        "capture": str(capture),
        "producer": str(producer),
        "metadata": meta,
        "steady_seconds": (end - start) / frequency,
        "target_hz": app["target_hz"],
        "produced_samples": len(samples),
        "changed_composed_frames": len(changed),
        "changed_frame_rate_hz": (
            (len(changed) - 1)
            * frequency
            / (changed[-1]["present_qpc"] - changed[0]["present_qpc"])
        ),
        "producer_rate_hz": (len(samples) - 1)
        * frequency
        / (samples[-1]["start_qpc"] - start),
        "producer_cpu": statistics_for([s["cpu_ms"] for s in samples]),
        "composed_frame_intervals": statistics_for(intervals),
        "intervals_over_33_34_ms": sum(v > 33.34 for v in intervals),
        "intervals_over_50_ms": sum(v > 50 for v in intervals),
        "observer_accumulated_updates": sum(
            max(0, r["accumulated"] - 1) for r in frames
        ),
        "observer_max_accumulated": max(r["accumulated"] for r in frames),
        "display_cycle_histogram": dict(
            sorted(
                collections.Counter(
                    round(v * refresh_hz / 1000) for v in intervals
                ).items()
            )
        ),
        "distinct_target_hashes": len(hashes),
        "most_common_hashes": hashes.most_common(25),
        "limitations": [
            "Composed desktop output, not panel scanout or remote stream delivery.",
            "Hash changes are not frame IDs; partial publications can also change pixels.",
            "Accumulated observer updates can hide intermediate composed images.",
            "Producer runs at target_hz; output intervals are quantized to monitor refresh.",
        ],
    }
    if samples[0].get("frame_id"):
        source = {s["frame_id"]: s for s in samples}
        if len(source) != len(samples):
            raise ValueError("Producer frame IDs must be unique")
        matched = {}
        for record in records:
            frame_id = record.get("frame_id", 0)
            if (
                frame_id in source
                and record["present_qpc"] >= source[frame_id]["start_qpc"]
            ):
                matched.setdefault(frame_id, record["present_qpc"])
        times = sorted(matched.values())
        result["matched_frame_ids"] = len(matched)
        result["missing_producer_frame_count"] = len(source) - len(matched)
        result["missing_producer_frame_ids"] = sorted(set(source) - set(matched))
        result["matched_frame_rate_hz"] = (
            ((len(times) - 1) * frequency / (times[-1] - times[0]))
            if len(times) > 1
            else 0
        )
        result["matched_frame_intervals"] = statistics_for(
            [(b - a) * 1000 / frequency for a, b in itertools.pairwise(times)]
        )
        result["matched_display_cycle_histogram"] = dict(
            sorted(
                collections.Counter(
                    round((b - a) * refresh_hz / frequency)
                    for a, b in itertools.pairwise(times)
                ).items()
            )
        )
        # Count complete one-second windows; exclude the partial last window.
        result["matched_frames_per_second"] = (
            [
                sum(
                    times[0] + second * frequency
                    <= t
                    < times[0] + (second + 1) * frequency
                    for t in times
                )
                for second in range(int((times[-1] - times[0]) / frequency))
            ]
            if times
            else []
        )
        result["matched_intervals_over_33_34_ms"] = sum(
            (b - a) * 1000 / frequency > 33.34 for a, b in itertools.pairwise(times)
        )
        result["update_to_composed_marker"] = statistics_for(
            [
                (time - source[frame_id]["start_qpc"]) * 1000 / frequency
                for frame_id, time in matched.items()
            ]
        )
    return result


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("capture")
    parser.add_argument("producer")
    parser.add_argument("output")
    args = parser.parse_args()
    result = analyze(args.capture, args.producer)
    Path(args.output).write_text(json.dumps(result, indent=2) + "\n", encoding="utf-8")
    print(
        json.dumps(
            {
                k: v
                for k, v in result.items()
                if k
                not in (
                    "most_common_hashes",
                    "limitations",
                    "missing_producer_frame_ids",
                )
            },
            indent=2,
        )
    )
