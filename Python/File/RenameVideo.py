# /*********************************************************************************************
# PROJECT:          Code Snippets
# COMPONENT:        Python/File
# FILE:             RenameVideo.py
# AUTHOR:           Ing. Mirko Mirabella
#                   m.mirabella@neptunengineering.com
#                   www.neptunengineering.com
# REVISION:         v. 1.1
# DATE:             23/09/2025
# **********************************************************************************************/

"""Rename video files using recording metadata.

The script scans a target directory, extracts the recording timestamp
from supported video files via pymediainfo, and renames each file using
that timestamp. When metadata is missing it falls back to the filesystem
creation time and guarantees unique filenames."""

from datetime import datetime
from pathlib import Path
from typing import Optional

from pymediainfo import MediaInfo


METADATA_DATE_FIELDS = (
    "recorded_date",
    "tagged_date",
    "encoded_date",
    "mastered_date",
    "creation_time",
)


def parse_metadata_datetime(raw_value: str) -> Optional[datetime]:
    """Try to normalise and parse a timestamp returned by MediaInfo."""
    if not raw_value:
        return None
    candidate = raw_value.strip()
    if not candidate:
        return None

    if candidate.upper().startswith("UTC "):
        candidate = candidate[4:].strip()
    if candidate.upper().endswith(" UTC"):
        candidate = candidate[:-4].strip()

    raw_candidates = [candidate]
    if candidate.endswith("Z"):
        raw_candidates.append(candidate[:-1] + "+00:00")

    normalised_candidates = []
    for value in raw_candidates:
        normalised_candidates.append(value)
        if "T" in value:
            normalised_candidates.append(value.replace("T", " "))

    seen = set()
    unique_candidates = []
    for value in normalised_candidates:
        value = value.strip()
        if not value:
            continue
        if value in seen:
            continue
        seen.add(value)
        unique_candidates.append(value)

    for value in unique_candidates:
        try:
            return datetime.fromisoformat(value)
        except ValueError:
            continue

    fallback_formats = (
        "%Y-%m-%d %H:%M:%S",
        "%Y/%m/%d %H:%M:%S",
        "%Y-%m-%d %H:%M:%S%z",
        "%Y%m%d %H%M%S",
    )
    for value in unique_candidates:
        for fmt in fallback_formats:
            try:
                return datetime.strptime(value, fmt)
            except ValueError:
                continue

    return None


def get_recording_data(path: Path) -> Optional[datetime]:
    """Return the first meaningful recording datetime found in the metadata."""
    try:
        media_info = MediaInfo.parse(str(path))
    except Exception as exc:
        print(f"Impossibile leggere i metadati di {path.name}: {exc}")
        return None

    for track in media_info.tracks:
        for field in METADATA_DATE_FIELDS:
            value = getattr(track, field, None)
            timestamp = parse_metadata_datetime(value) if value else None
            if timestamp:
                return timestamp

    return None


def format_timestamp(dt: datetime) -> str:
    """Return the timestamp string used in the renamed file."""
    if dt.tzinfo:
        dt = dt.astimezone()
    return dt.strftime("%d%m%Y_%H%M%S")


def rename_videos_with_creation_date(folder_path: str, label: str = "") -> None:
    folder = Path(folder_path).expanduser()
    if not folder.is_dir():
        raise ValueError(f"{folder_path} non e una directory valida.")

    video_exts = {".mp4", ".mov", ".avi", ".mkv", ".flv", ".wmv"}
    label_prefix = label or ""
    if label_prefix and not label_prefix.endswith("_"):
        label_prefix = f"{label_prefix}_"

    for file_path in folder.iterdir():
        if not file_path.is_file():
            continue
        if file_path.suffix.lower() not in video_exts:
            continue

        metadata_dt = get_recording_data(file_path)
        source = "metadata"
        if metadata_dt is None:
            source = "filesystem"
            metadata_dt = datetime.fromtimestamp(file_path.stat().st_ctime)

        timestamp_str = format_timestamp(metadata_dt)
        base_name = f"{label_prefix}{timestamp_str}"
        new_path = file_path.with_name(f"{base_name}{file_path.suffix}")

        counter = 1
        while new_path.exists():
            new_path = file_path.with_name(f"{base_name}_{counter}{file_path.suffix}")
            counter += 1

        try:
            print(f"Rinomino {file_path.name} -> {new_path.name} (origine: {source})")
            file_path.rename(new_path)
        except Exception as exc:
            print(f"Errore rinominando {file_path.name}: {exc}")


if __name__ == "__main__":
    folder = input("Percorso della cartella con i video: ").strip()
    label = input("Etichetta da aggiungere (opzionale): ").strip()
    rename_videos_with_creation_date(folder, label)
