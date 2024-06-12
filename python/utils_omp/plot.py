import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import seaborn as sns

fontsize=20
width=8
height=6

# Time vs Threads plot
def timeplot(df):
    plt.figure(figsize=(width,height))

    # Set Seaborn theme, grid aesthetic and Matplotlib background and figure size
    sns.set_theme(style="whitegrid", palette="tab10", rc={"grid.linestyle": "--", "grid.color": "gray", "grid.alpha": 0.3, "grid.linewidth": 0.5})

    # Average time for every method for each value of "p"
    task_df = df[df["method"] == "task"].groupby(["th"])["t"].mean().reset_index()
    simple_df = df[df["method"] == "simple"].groupby(["th"])["t"].mean().reset_index()
    hyper_df = df[df["method"] == "hyper"].groupby(["th"])["t"].mean().reset_index()
    psrs_df = df[df["method"] == "psrs"].groupby(["th"])["t"].mean().reset_index()
    serial_time = df[df["method"] == "serial"]["t"].mean()

    # Average standard deviation for every method for each value of "p"
    task_df["std"] = df[df["method"] == "task"].groupby(["th"])["std"].mean().reset_index()["std"]
    simple_df["std"] = df[df["method"] == "simple"].groupby(["th"])["std"].mean().reset_index()["std"]
    hyper_df["std"] = df[df["method"] == "hyper"].groupby(["th"])["std"].mean().reset_index()["std"]
    psrs_df["std"] = df[df["method"] == "psrs"].groupby(["th"])["std"].mean().reset_index()["std"]

    # Create the lineplot for every method
    plt.axhline(y=serial_time, linestyle='--', label='Serial time', linewidth=1.5, color='tab:green')
    sns.lineplot(data=task_df, x="th", y="t", label="Task", linewidth=1, marker='o', markersize=4, color='#0d007e')
    sns.lineplot(data=simple_df, x="th", y="t", label="Simple", linewidth=1, marker='o', markersize=4, color='tab:orange')
    sns.lineplot(data=hyper_df, x="th", y="t", label="Hyper", linewidth=1, marker='o', markersize=4, color='tab:purple')
    sns.lineplot(data=psrs_df, x="th", y="t", label="PSRS", linewidth=1, marker='o', markersize=4, color='tab:red')

    # Add vertical dotted gray lines for every multiple of 2 smaller than the maximum number of processes
    # for i in range(1, int(np.log2(max(df["th"])))+1):
    #     plt.axvline(x=2**i, color='gray', linestyle='--', linewidth=0.7)
        # plt.text(2**i-0.01, -0.01, f'{2**i}', verticalalignment='center', horizontalalignment='right', fontsize=12, color='gray')

    # Add error bars for every multiple of 2 smaller than the maximum number of processes using the std column
    for i in range(1, int(np.log2(max(df["th"])))+1):
        plt.errorbar(task_df['th'], task_df['t'], yerr=task_df['std'], fmt='none', color='#0d007e', elinewidth=0.1, capsize=1.5)
        plt.errorbar(simple_df['th'], simple_df['t'], yerr=simple_df['std'], fmt='none', color='tab:orange', elinewidth=0.1, capsize=1.5)
        plt.errorbar(hyper_df['th'], hyper_df['t'], yerr=hyper_df['std'], fmt='none', color='tab:purple', elinewidth=0.1, capsize=1.5)
        plt.errorbar(psrs_df['th'], psrs_df['t'], yerr=psrs_df['std'], fmt='none', color='tab:red', elinewidth=0.1, capsize=1.5)

    # --- log2 scale ---
    plt.xscale('log', base=2)
    # plt.yscale('log', base=2)

    # Set plot title and labels
    n = df['n'].iloc[0]
    plt.title(f"OpenMP Average Time ({n:.0e} elements)", fontsize=fontsize)

    x_ticks = [2**i for i in range(1, int(np.log2(max(df["th"])))+1)]
    plt.xticks(x_ticks, x_ticks)
    plt.xlabel('Number of Threads (Log2 scale)', fontsize=fontsize)
    plt.ylabel('Average Time (s)', fontsize=fontsize)
    plt.ylim(0, max(df["t"])+1)
    plt.ylim(0, 25)
    # plt.ylim(0, (1.1)*serial_time)
    plt.legend(loc = 'upper left', framealpha=1, fontsize=16)

    # Save the image as high quality pdf
    plt.savefig("omp_timeplot.pdf", format='pdf', dpi=300, bbox_inches='tight')

    plt.show()

# Speedup vs Threads plot
def speedplot(df):
    plt.figure(figsize=(width,height))

    # Set Seaborn theme, grid aesthetic and Matplotlib background and figure size
    sns.set_theme(style="whitegrid", palette="tab10", rc={"grid.linestyle": "--", "grid.color": "gray", "grid.alpha": 0.3, "grid.linewidth": 0.5})

    # Average speedup for every method for each value of "p"
    task_df = df[df["method"] == "task"].groupby(["th"])["sp"].mean().reset_index()
    simple_df = df[df["method"] == "simple"].groupby(["th"])["sp"].mean().reset_index()
    hyper_df = df[df["method"] == "hyper"].groupby(["th"])["sp"].mean().reset_index()
    psrs_df = df[df["method"] == "psrs"].groupby(["th"])["sp"].mean().reset_index()

    # Average standard deviation for every method for each value of "p"
    task_df["sp_std"] = df[df["method"] == "task"].groupby(["th"])["sp_std"].mean().reset_index()["sp_std"]
    simple_df["sp_std"] = df[df["method"] == "simple"].groupby(["th"])["sp_std"].mean().reset_index()["sp_std"]
    hyper_df["sp_std"] = df[df["method"] == "hyper"].groupby(["th"])["sp_std"].mean().reset_index()["sp_std"]
    psrs_df["sp_std"] = df[df["method"] == "psrs"].groupby(["th"])["sp_std"].mean().reset_index()["sp_std"]

    # Add vertical dotted gray lines for every multiple of 2 smaller than the maximum number of processes
    for i in range(1, int(np.log2(max(df["th"])))+1):
        plt.axvline(x=2**i, color='tab:gray', linestyle=':', linewidth=1, alpha=0.7)
        if i > 2:
            plt.text(2**i-0.1, 0, f'{2**i}', verticalalignment='bottom', horizontalalignment='right', fontsize=12, color='tab:gray')

    # Create the lineplot
    # sns.lineplot(data=df, x="p", y="sp", hue="method", linewidth=1, marker='o', markersize=5, errorbar=None, legend=True)
    sns.lineplot(data=task_df, x="th", y="sp", label="Task", linewidth=1, marker='o', markersize=4, color='#0d007e')
    sns.lineplot(data=simple_df, x="th", y="sp", label="Simple", linewidth=1, marker='o', markersize=4, color='tab:orange')
    sns.lineplot(data=hyper_df, x="th", y="sp", label="Hyper", linewidth=1, marker='o', markersize=4, color='tab:purple')
    sns.lineplot(data=psrs_df, x="th", y="sp", label="PSRS", linewidth=1, marker='o', markersize=4, color='tab:red')

    # Ideal speedup
    plt.plot([0, max(df["th"])], [0, max(df["th"])], color='tab:green', linestyle='--', label='Ideal', linewidth=0.7)

    # Add error bars for every multiple of 2 smaller than the maximum number of processes using the std column
    for i in range(1, int(np.log2(max(df["th"])))+1):
        plt.errorbar(task_df['th'], task_df['sp'], yerr=task_df['sp_std'], fmt='none', color='#0d007e', elinewidth=0.1, capsize=1.5)
        plt.errorbar(simple_df['th'], simple_df['sp'], yerr=simple_df['sp_std'], fmt='none', color='tab:orange', elinewidth=0.1, capsize=1.5)
        plt.errorbar(hyper_df['th'], hyper_df['sp'], yerr=hyper_df['sp_std'], fmt='none', color='tab:purple', elinewidth=0.1, capsize=1.5)
        plt.errorbar(psrs_df['th'], psrs_df['sp'], yerr=psrs_df['sp_std'], fmt='none', color='tab:red', elinewidth=0.1, capsize=1.5)

    # Add vertical dotted gray lines for every multiple of 2 smaller than the maximum number of processes
    for i in range(1, int(np.log2(max(df["th"])))+1):
        plt.axvline(x=2**i, color='tab:gray', linestyle=':', linewidth=1, alpha=0.7)
        if i > 2:
            plt.text(2**i-0.1, 0, f'{2**i}', verticalalignment='bottom', horizontalalignment='right', fontsize=12, color='tab:gray')

    # --- log2 scale ---
    # plt.xscale('log', base=2)
    # plt.yscale('log', base=2)

    # Set plot title and labels
    n = df['n'].iloc[0]
    plt.title(f"OpenMP Speedup ({n:.0e} elements)", fontsize=fontsize)
    plt.xlabel('Number of Threads', fontsize=fontsize)
    plt.ylabel('Speedup', fontsize=fontsize)
    plt.xticks(np.arange(0, max(df["th"])+3, 10))
    plt.xlim(0, max(df["th"])+2)
    plt.ylim(0, max(df["sp"])+1)
    plt.legend(loc = 'upper right', framealpha=1, fontsize=16, ncol=2)

    # Save the image as high quality pdf
    plt.savefig("omp_speedplot.pdf", format='pdf', dpi=300, bbox_inches='tight')

    plt.show()

# Weak Time vs Processes plot
def weaktimeplot(df):
    plt.figure(figsize=(width,height))

    # Set Seaborn theme, grid aesthetic and Matplotlib background and figure size
    sns.set_theme(style="whitegrid", palette="tab10", rc={"grid.linestyle": "--", "grid.color": "gray", "grid.alpha": 0.3, "grid.linewidth": 0.5})

    # Average time for every method for each value of "th"
    task_df = df[df["method"] == "task"].groupby(["th"])["t"].mean().reset_index()
    simple_df = df[df["method"] == "simple"].groupby(["th"])["t"].mean().reset_index()
    hyper_df = df[df["method"] == "hyper"].groupby(["th"])["t"].mean().reset_index()
    psrs_df = df[df["method"] == "psrs"].groupby(["th"])["t"].mean().reset_index()
    # serial_time = df[df["method"] == "serial"]["t"].mean()

    # Average standard deviation for every method for each value of "th"
    task_df["std"] = df[df["method"] == "task"].groupby(["th"])["std"].mean().reset_index()["std"]
    simple_df["std"] = df[df["method"] == "simple"].groupby(["th"])["std"].mean().reset_index()["std"]
    hyper_df["std"] = df[df["method"] == "hyper"].groupby(["th"])["std"].mean().reset_index()["std"]
    psrs_df["std"] = df[df["method"] == "psrs"].groupby(["th"])["std"].mean().reset_index()["std"]

    # Add vertical dotted gray lines for every multiple of 2 smaller than the maximum number of processes
    for i in range(1, int(np.log2(max(df["th"])))+1):
        plt.axvline(x=2**i, color='tab:gray', linestyle=':', linewidth=1, alpha=0.7)
        if i > 2:
            plt.text(2**i-0.1, +0.1, f'{2**i}', verticalalignment='bottom', horizontalalignment='right', fontsize=12, color='tab:gray')

    # Create the lineplot
    # sns.lineplot(data=df, x="p", y="t", hue="method", linewidth=1, marker='o', markersize=5, errorbar=None, legend=True)
    sns.lineplot(data=task_df, x="th", y="t", label="Task", linewidth=1, marker='o', markersize=4, color='#0d007e')
    sns.lineplot(data=simple_df, x="th", y="t", label="Simple", linewidth=1, marker='o', markersize=4, color='tab:orange')
    sns.lineplot(data=hyper_df, x="th", y="t", label="Hyper", linewidth=1, marker='o', markersize=4, color='tab:purple')
    sns.lineplot(data=psrs_df, x="th", y="t", label="PSRS", linewidth=1, marker='o', markersize=4, color='tab:red')
    # plt.axhline(y=serial_time*max(df['p']), linestyle='--', label='Serial time', linewidth=1, color='tab:red')

    # Add error bars for every multiple of 2 smaller than the maximum number of processes using the std column
    for i in range(1, int(np.log2(max(df["th"])))+1):
        plt.errorbar(task_df['th'], task_df['t'], yerr=task_df['std'], fmt='none', color='#0d007e', elinewidth=0.1, capsize=1.5)
        plt.errorbar(simple_df['th'], simple_df['t'], yerr=simple_df['std'], fmt='none', color='tab:orange', elinewidth=0.1, capsize=1.5)
        plt.errorbar(hyper_df['th'], hyper_df['t'], yerr=hyper_df['std'], fmt='none', color='tab:purple', elinewidth=0.1, capsize=1.5)
        plt.errorbar(psrs_df['th'], psrs_df['t'], yerr=psrs_df['std'], fmt='none', color='tab:red', elinewidth=0.1, capsize=1.5)

    # --- log2 scale ---
    # plt.xscale('log', base=2)
    # plt.yscale('log', base=2)

    # Set plot title and labels
    n = df['n'].iloc[0]
    th = df['th'].iloc[0]
    plt.title(f"OpenMP Average Time ({n/th:.0e} elements per thread)", fontsize=fontsize)

    # x_ticks = [2**i for i in range(0, int(np.log2(max(df["p"])))+1)]
    # plt.xticks(x_ticks, x_ticks)
    plt.xlabel('Number of Threads', fontsize=fontsize)
    plt.ylabel('Average Time (s)', fontsize=fontsize)
    plt.xticks(np.arange(0, max(df["th"])+3, 10))
    plt.xlim(0, max(df["th"])+2)
    plt.ylim(0, max(df["t"])+5)
    plt.legend(loc = 'upper left', framealpha=1, fontsize=16)

    # Save the image as high quality pdf
    plt.savefig("omp_weaktimeplot.pdf", format='pdf', dpi=300, bbox_inches='tight')

    plt.show()

# Efficiency vs Processes plot
def effplot(df):
    plt.figure(figsize=(width,height))

    # Set Seaborn theme, grid aesthetic and Matplotlib background and figure size
    sns.set_theme(style="whitegrid", palette="tab10", rc={"grid.linestyle": "--", "grid.color": "gray", "grid.alpha": 0.3, "grid.linewidth": 0.5})

    # Average efficiency for every method for each value of "p"
    task_df = df[df["method"] == "task"].groupby(["th"])["eff"].mean().reset_index()
    simple_df = df[df["method"] == "simple"].groupby(["th"])["eff"].mean().reset_index()
    hyper_df = df[df["method"] == "hyper"].groupby(["th"])["eff"].mean().reset_index()
    psrs_df = df[df["method"] == "psrs"].groupby(["th"])["eff"].mean().reset_index()

    # Average standard deviation for every method for each value of "p"
    task_df["eff_std"] = df[df["method"] == "task"].groupby(["th"])["eff_std"].mean().reset_index()["eff_std"]
    simple_df["eff_std"] = df[df["method"] == "simple"].groupby(["th"])["eff_std"].mean().reset_index()["eff_std"]
    hyper_df["eff_std"] = df[df["method"] == "hyper"].groupby(["th"])["eff_std"].mean().reset_index()["eff_std"]
    psrs_df["eff_std"] = df[df["method"] == "psrs"].groupby(["th"])["eff_std"].mean().reset_index()["eff_std"]

    # Create the lineplot
    # sns.lineplot(data=df, x="p", y="eff", hue="method", linewidth=1, marker='o', markersize=5, errorbar=None, legend=True)
    sns.lineplot(data=task_df, x="th", y="eff", label="Task", linewidth=1, marker='o', markersize=4, color='#0d007e')
    sns.lineplot(data=simple_df, x="th", y="eff", label="Simple", linewidth=1, marker='o', markersize=4, color='tab:orange')
    sns.lineplot(data=hyper_df, x="th", y="eff", label="Hyper", linewidth=1, marker='o', markersize=4, color='tab:purple')
    sns.lineplot(data=psrs_df, x="th", y="eff", label="PSRS", linewidth=1, marker='o', markersize=4, color='tab:red')

    # Ideal efficiency
    plt.axhline(y=1, color='tab:green', linestyle='--', label='Ideal', linewidth=1.5)

    # Add vertical dotted gray lines for every multiple of 2 smaller than the maximum number of processes
    # for i in range(1, int(np.log2(max(df["p"])))+1):
    #     plt.axvline(x=2**i, color='tab:gray', linestyle=':', linewidth=1, alpha=0.7)
    #     if i > 2:
    #         plt.text(2**i-0.1, 1.1, f'{2**i}', verticalalignment='top', horizontalalignment='right', fontsize=12, color='tab:gray')

    # Add error bars for every multiple of 2 smaller than the maximum number of processes using the std column
    for i in range(1, int(np.log2(max(df["th"])))+1):
        plt.errorbar(task_df['th'], task_df['eff'], yerr=task_df['eff_std'], fmt='none', color='#0d007e', elinewidth=0.1, capsize=1.5)
        plt.errorbar(simple_df['th'], simple_df['eff'], yerr=simple_df['eff_std'], fmt='none', color='tab:orange', elinewidth=0.1, capsize=1.5)
        plt.errorbar(hyper_df['th'], hyper_df['eff'], yerr=hyper_df['eff_std'], fmt='none', color='tab:purple', elinewidth=0.1, capsize=1.5)
        plt.errorbar(psrs_df['th'], psrs_df['eff'], yerr=psrs_df['eff_std'], fmt='none', color='tab:red', elinewidth=0.1, capsize=1.5)

    # --- log2 scale ---
    # plt.xscale('log', base=2)
    # plt.yscale('log', base=2)

    # Set plot title and labels
    n = df['n'].iloc[0]
    plt.title(f"OpenMP Efficiency ({n:.0e} elements per thread)", fontsize=fontsize)
    # Ad  vertical dotted gray lines for every multiple of 2 smaller than the maximum number of processes
    for i in range(1, int(np.log2(max(df["th"])))+1):
        plt.axvline(x=2**i, color='tab:gray', linestyle=':', linewidth=1, alpha=0.7)
        if i > 2:
            plt.text(2**i-0.1, 0, f'{2**i}', verticalalignment='bottom', horizontalalignment='right', fontsize=12, color='tab:gray')


    # x_ticks = [2**i for i in range(0, int(np.log2(max(df["p"])))+1)]
    # plt.xticks(x_ticks, x_ticks)

    y_ticks = [0.1*i for i in range(0, 11)]
    plt.yticks(y_ticks, [f'{i:.0%}' for i in y_ticks])

    plt.xlabel('Number of Threads', fontsize=fontsize)
    plt.ylabel('Efficiency', fontsize=fontsize)
    # plt.xlim(0, max(df["p"])+1)
    plt.xticks(np.arange(0, max(df["th"])+3, 10))
    plt.xlim(0, max(df["th"])+2)
    plt.ylim(0, 1.1)
    plt.legend(loc = 'upper right', framealpha=1, fontsize=16)

    # Save the image as high quality pdf
    plt.savefig("omp_effplot.pdf", format='pdf', dpi=300, bbox_inches='tight')

    plt.show()