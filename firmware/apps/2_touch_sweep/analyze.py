#!/usr/bin/env python3
"""
Touch Sweep — Análisis de firmas capacitivas (8-config sweep).

Lee archivos .txt exportados desde el ESP32 (modo #STAT), grafica las
firmas crudas y evalúa cuál de las 8 configuraciones de excitación tiene
mejor relación señal/ruido para detección de contacto.

Cada archivo .txt corresponde a una condición (sin contacto, con contacto, etc.).
Un archivo puede contener múltiples bloques SWEEP STATS (reps de la misma condición).

Uso:
    python analyze.py contacto.txt sin_contacto.txt
    python analyze.py *.txt
    python analyze.py --dir ./datos/
"""

import sys
import re
import argparse
import numpy as np
import matplotlib.pyplot as plt
from pathlib import Path

# Labels de las 8 configuraciones (deben coincidir con el firmware)
STEP_LABELS = [
    "s1_L",    # speed=1, LOW
    "s1_F",    # speed=1, FLOAT
    "s1_H",    # speed=1, HIGH
    "s7_L",    # speed=7, LOW
    "s7_F",    # speed=7, FLOAT
    "s7_H",    # speed=7, HIGH
    "s4_F",    # speed=4, FLOAT (mid)
    "def",     # speed=7, DEFAULT (baseline HAL)
]


def parse_stat_file(filepath):
    """
    Extrae TODOS los bloques SWEEP STATS de un archivo .txt con formato:
        --- SWEEP STATS (N steps) ---
        Mean: X | StdDev: Y | Min: A | Max: B | CV: Z%
        raw:v1,v2,v3,...
    Retorna lista de dicts (un dict por sweep encontrado).
    """
    with open(filepath, "r") as f:
        content = f.read()

    blocks = re.split(r"---+\s*SWEEP STATS", content)
    results = []

    for block in blocks:
        if not block.strip():
            continue

        meta = {}

        m = re.search(r"\((\d+) steps\)", block)
        if m:
            meta["steps"] = int(m.group(1))

        for key in ["Mean", "StdDev", "Min", "Max", "CV"]:
            m = re.search(rf"{key}:\s*([\d.]+)", block)
            if m:
                meta[key.lower()] = float(m.group(1))

        raw_matches = re.findall(r"raw:([\d,\s]+)", block)
        if not raw_matches:
            continue

        all_values = []
        for rm in raw_matches:
            vals = [int(x.strip()) for x in rm.split(",") if x.strip()]
            all_values.extend(vals)

        if all_values:
            meta["raw"] = np.array(all_values, dtype=np.float64)
            results.append(meta)

    if not results:
        raise ValueError(f"No se encontró ningún bloque SWEEP STATS en {filepath}")

    return results


def load_conditions(filepaths):
    """
    Carga múltiples archivos. Cada archivo = una condición.
    Retorna dict: {nombre_condicion: [list de sweeps (np.array)]}
    """
    conditions = {}
    for fp in filepaths:
        name = Path(fp).stem
        sweep_list = parse_stat_file(fp)
        if name not in conditions:
            conditions[name] = []
        for data in sweep_list:
            conditions[name].append(data["raw"])
        n_steps = sweep_list[0].get("steps", len(sweep_list[0]["raw"]))
        print(f"  [{name}] {len(sweep_list)} sweeps × {n_steps} steps")
    return conditions


def compute_stats(sweeps):
    """
    Dada una lista de arrays de una misma condición,
    calcula estadísticas por step y globales.
    """
    matrix = np.vstack(sweeps)
    result = {
        "mean_per_step": np.mean(matrix, axis=0),
        "std_per_step": np.std(matrix, axis=0),
        "min_per_step": np.min(matrix, axis=0),
        "max_per_step": np.max(matrix, axis=0),
        "cv_per_step": np.where(
            np.mean(matrix, axis=0) > 0,
            np.std(matrix, axis=0) / np.mean(matrix, axis=0) * 100,
            0
        ),
        "global_mean": np.mean(matrix),
        "global_std": np.std(matrix),
        "global_cv": np.std(matrix) / np.mean(matrix) * 100 if np.mean(matrix) > 0 else 0,
        "n_sweeps": len(sweeps),
        "n_steps": matrix.shape[1],
        "matrix": matrix,
    }
    return result


def plot_firmas(cond_stats):
    """
    Gráfica 1: Firmas promedio de cada condición superpuestas.
    """
    fig, axes = plt.subplots(2, 1, figsize=(12, 8), sharex=True)

    ax = axes[0]
    for name, stats in cond_stats.items():
        x = np.arange(stats["n_steps"])
        ax.plot(x, stats["mean_per_step"], "o-", label=name, linewidth=1.5,
                markersize=5)
        ax.fill_between(
            x,
            stats["mean_per_step"] - stats["std_per_step"],
            stats["mean_per_step"] + stats["std_per_step"],
            alpha=0.15
        )
    ax.set_ylabel("Raw value (promedio ± 1σ)")
    ax.set_title("Firmas por condición — 8 configuraciones de excitación")
    ax.set_xticks(range(len(STEP_LABELS)))
    ax.set_xticklabels(STEP_LABELS, rotation=45, ha="right")
    ax.legend()
    ax.grid(True, alpha=0.3)

    ax = axes[1]
    for name, stats in cond_stats.items():
        ax.plot(np.arange(stats["n_steps"]), stats["cv_per_step"],
                "o-", label=name, linewidth=1.2, markersize=4)
    ax.set_xlabel("Configuración de excitación")
    ax.set_ylabel("CV% intra-condición")
    ax.set_title("Variabilidad entre sweeps de la misma condición")
    ax.set_xticks(range(len(STEP_LABELS)))
    ax.set_xticklabels(STEP_LABELS, rotation=45, ha="right")
    ax.legend()
    ax.grid(True, alpha=0.3)

    plt.tight_layout()
    plt.savefig("firmas.png", dpi=150)
    plt.show()
    print("Guardado: firmas.png")


def plot_heatmap(cond_stats):
    """
    Gráfica 2: Heatmap de la matrix cruda de cada condición.
    """
    n_conds = len(cond_stats)
    fig, axes = plt.subplots(1, n_conds, figsize=(5 * n_conds, 5), sharey=True)
    if n_conds == 1:
        axes = [axes]

    for ax, (name, stats) in zip(axes, cond_stats.items()):
        im = ax.imshow(stats["matrix"], aspect="auto", cmap="viridis",
                       interpolation="nearest")
        ax.set_title(f"{name}\n({stats['n_sweeps']} sweeps)")
        ax.set_xlabel("Step")
        ax.set_ylabel("Sweep #")
        ax.set_xticks(range(len(STEP_LABELS)))
        ax.set_xticklabels(STEP_LABELS, rotation=45, ha="right", fontsize=8)
        plt.colorbar(im, ax=ax, shrink=0.8)

    plt.tight_layout()
    plt.savefig("heatmaps.png", dpi=150)
    plt.show()
    print("Guardado: heatmaps.png")


def plot_snr(cond_stats):
    """
    Gráfica 3: Relación señal/ruido por configuración.
    Señal = distancia entre centroides de cada par de condiciones.
    Ruido = variabilidad intra-condición (std promedio por step).
    """
    names = list(cond_stats.keys())
    n_steps = min(s["n_steps"] for s in cond_stats.values())
    labels = STEP_LABELS[:n_steps]

    # Para cada condición, std promedio por step (ruido)
    noise = {}
    for name, stats in cond_stats.items():
        noise[name] = stats["std_per_step"][:n_steps]

    # Para cada par de condiciones, calcular S/N por step
    fig, axes = plt.subplots(2, 1, figsize=(12, 8), sharex=True)

    # Subplot 1: Señal (distancia entre pares por step)
    ax = axes[0]
    pairs = []
    for i in range(len(names)):
        for j in range(i + 1, len(names)):
            a = cond_stats[names[i]]["mean_per_step"][:n_steps]
            b = cond_stats[names[j]]["mean_per_step"][:n_steps]
            signal = np.abs(a - b)
            pairs.append((f"{names[i]} vs {names[j]}", signal))
            ax.plot(signal, "o-", label=f"{names[i]} vs {names[j]}",
                    linewidth=1.5, markersize=5)
    ax.set_ylabel("Δ raw (señal)")
    ax.set_title("Separación entre condiciones por configuración")
    ax.set_xticks(range(n_steps))
    ax.set_xticklabels(labels, rotation=45, ha="right")
    ax.legend(fontsize=8)
    ax.grid(True, alpha=0.3)

    # Subplot 2: Ruido (std intra-condición promedio)
    ax = axes[1]
    for name in names:
        ax.plot(noise[name], "o-", label=name, linewidth=1.5, markersize=5)
    ax.set_ylabel("StdDev (ruido intra-condición)")
    ax.set_xlabel("Configuración de excitación")
    ax.set_title("Variabilidad intra-condición por configuración")
    ax.set_xticks(range(n_steps))
    ax.set_xticklabels(labels, rotation=45, ha="right")
    ax.legend(fontsize=8)
    ax.grid(True, alpha=0.3)

    plt.tight_layout()
    plt.savefig("snr.png", dpi=150)
    plt.show()
    print("Guardado: snr.png")


def plot_snr_ratio(cond_stats):
    """
    Gráfica 4: Ratio S/N por configuración.
    Para cada par de condiciones, S/N = mean(distancia) / mean(std).
    """
    names = list(cond_stats.keys())
    n_steps = min(s["n_steps"] for s in cond_stats.values())
    labels = STEP_LABELS[:n_steps]

    if len(names) < 2:
        print("  [skip] Se necesitan ≥2 condiciones para calcular S/N ratio")
        return

    fig, ax = plt.subplots(figsize=(10, 5))

    for i in range(len(names)):
        for j in range(i + 1, len(names)):
            a = cond_stats[names[i]]["mean_per_step"][:n_steps]
            b = cond_stats[names[j]]["mean_per_step"][:n_steps]
            std_a = cond_stats[names[i]]["std_per_step"][:n_steps]
            std_b = cond_stats[names[j]]["std_per_step"][:n_steps]

            signal = np.abs(a - b)
            noise = (std_a + std_b) / 2
            noise = np.where(noise > 0, noise, 1)  # evitar div/0
            snr = signal / noise

            ax.plot(snr, "o-", label=f"{names[i]} vs {names[j]}",
                    linewidth=1.5, markersize=6)

    ax.set_xlabel("Configuración de excitación")
    ax.set_ylabel("S/N ratio")
    ax.set_title("Relación señal/ruido por configuración (mayor = mejor)")
    ax.set_xticks(range(n_steps))
    ax.set_xticklabels(labels, rotation=45, ha="right")
    ax.axhline(y=1, color="r", linestyle="--", alpha=0.5, label="S/N=1 (umbral)")
    ax.legend(fontsize=8)
    ax.grid(True, alpha=0.3)

    plt.tight_layout()
    plt.savefig("snr_ratio.png", dpi=150)
    plt.show()
    print("Guardado: snr_ratio.png")


def clasificacion_simple(cond_stats):
    """
    Análisis de separabilidad: leave-one-out entre condiciones.
    """
    names = list(cond_stats.keys())
    n_steps = min(s["n_steps"] for s in cond_stats.values())

    print("\n" + "=" * 60)
    print("  ANÁLISIS DE SEPARABILIDAD")
    print("=" * 60)

    centroids = {}
    for name, stats in cond_stats.items():
        c = stats["mean_per_step"][:n_steps]
        centroids[name] = c / np.max(c) if np.max(c) > 0 else c

    for name in names:
        dists = {}
        for other in names:
            dists[other] = np.linalg.norm(centroids[name] - centroids[other])

        ranked = sorted(dists.items(), key=lambda x: x[1])
        closest = ranked[0][0]
        status = "OK" if closest == name else "FALLO"

        detail = " | ".join(f"{n}={d:.3f}" for n, d in ranked)
        print(f"  {name:20s} → {closest:20s} [{status}]  ({detail})")

    print("=" * 60)


def main():
    parser = argparse.ArgumentParser(
        description="Análisis de firmas capacitivas de Touch Sweep (8-config)")
    parser.add_argument("files", nargs="*", help="Archivos .txt por condición")
    parser.add_argument("--dir", type=str, default=None,
                        help="Directorio con archivos .txt")
    args = parser.parse_args()

    filepaths = []
    if args.dir:
        for f in sorted(Path(args.dir).glob("*.txt")):
            filepaths.append(str(f))
    if args.files:
        filepaths.extend(args.files)

    if not filepaths:
        print("Error: indicar archivos o usar --dir")
        sys.exit(1)

    print(f"\nCargando {len(filepaths)} archivos...\n")
    conditions = load_conditions(filepaths)

    cond_stats = {}
    for name, sweeps in conditions.items():
        cond_stats[name] = compute_stats(sweeps)
        s = cond_stats[name]
        print(f"  [{name}] {s['n_sweeps']} sweeps × {s['n_steps']} steps | "
              f"mean={s['global_mean']:.1f}, cv={s['global_cv']:.1f}%")

    plot_firmas(cond_stats)
    plot_heatmap(cond_stats)
    plot_snr(cond_stats)
    plot_snr_ratio(cond_stats)
    clasificacion_simple(cond_stats)


if __name__ == "__main__":
    main()
