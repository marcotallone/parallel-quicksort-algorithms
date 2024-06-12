import pandas as pd
import numpy as np

def preproc(filename):
    # Load a dataset from csv file
    df = pd.read_csv(filename)

    # Check if in the "Sorted" column all values are "Yes"
    if not all(df["Sorted"] == "Yes"):
        print(df[df["Sorted"] != "Yes"])

    # Drop the "Sorted" column
    df = df.drop(columns=["Sorted"])

    # Convert columns to the appropiate datatypes
    df["Threads"] = df["Threads"].astype(int)
    df["Elements"] = df["Elements"].astype(int)
    df["Elements (Scientific)"] = df["Elements (Scientific)"].astype(int)
    df["Average time (s)"] = df["Average time (s)"].astype(float)
    df["St. Deviation (s)"] = df["St. Deviation (s)"].astype(float)
    df["Min (s)"] = df["Min (s)"].astype(float)
    df["Max (s)"] = df["Max (s)"].astype(float)

    # Convert column names 
    df = df.rename(columns={"Threads": "th"})
    df = df.rename(columns={"Elements": "n"})
    df = df.rename(columns={"Elements (Scientific)": "n_sci"})
    df = df.rename(columns={"Average time (s)": "t"})
    df = df.rename(columns={"St. Deviation (s)": "std"})
    df = df.rename(columns={"Min (s)": "min"})
    df = df.rename(columns={"Max (s)": "max"})
    df = df.rename(columns={"Method": "method"})

    # Find all the unique values in the "n" column
    ns = df["n"].unique()

    # Find the serial time for each value of "n"
    serial_time = {}
    serial_std = {}
    for n in ns:
        serial_time[n] = df[(df["th"] == 1) & (df["n"] == n)]["t"].mean()
        serial_std[n] = df[(df["th"] == 1) & (df["n"] == n)]["std"].mean()

    # Calculate the speedup and efficiency for each value of "n"
    for n in ns:
        df.loc[df["n"] == n, "sp"] = serial_time[n] / df[df["n"] == n]["t"]
        df.loc[(df["n"] / df["th"]) == n, "eff"] = serial_time[n] / df[(df["n"] / df["th"]) == n]["t"]

        df.loc[df["n"] == n, "sp_std"] = ( (serial_std[n]**2 / df[df["n"] == n]["t"]**2)
                                          + (df[df["n"] == n]["std"]**2 * serial_time[n]**2 / df[df["n"] == n]["t"]**4) ).apply(np.sqrt)

        df.loc[(df["n"] / df["th"]) == n, "eff_std"] = ( (serial_std[n]**2 / df[(df["n"] / df["th"]) == n]["t"]**2)
                                            + (df[(df["n"] / df["th"]) == n]["std"]**2 * serial_time[n]**2 / df[(df["n"] / df["th"]) == n]["t"]**4) ).apply(np.sqrt)
    return df
