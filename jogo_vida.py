from pyspark.sql import SparkSession
from pyspark.sql.functions import col, udf
from pyspark.sql.types import IntegerType
import pandas as pd
import time

# Define the wall_time function to measure time
def wall_time():
    return time.time()

# Define the UmaVida function to perform one iteration of the life simulation
def update_cell(row_idx, col_idx, val, tabulIn):
    # Update the cell's value based on the number of live neighbors
    vizviv = get_vizviv(row_idx, col_idx, tabulIn)
    if val and vizviv < 2:
        return 0
    elif val and vizviv > 3:
        return 0
    elif not val and vizviv == 3:
        return 1
    else:
        return val

def uma_vida(tabulIn_df):
    # Implement the life simulation logic for one iteration
    max_row = tabulIn_df.agg({"row": "max"}).collect()[0][0]
    return [
        (row, col, update_cell(tabulIn_df, row, col))
        for row in range(1, max_row + 1)
        for col in range(1, max_row + 1)
    ]

def get_vizviv(row_idx, col_idx, tabulIn_df):
    # Calculate the number of live neighbors for a given cell
    row_above = tabulIn_df[(tabulIn_df['row'] == row_idx - 1) & ((tabulIn_df['col'] == col_idx - 1) | (tabulIn_df['col'] == col_idx) | (tabulIn_df['col'] == col_idx + 1))]
    row_below = tabulIn_df[(tabulIn_df['row'] == row_idx + 1) & ((tabulIn_df['col'] == col_idx - 1) | (tabulIn_df['col'] == col_idx) | (tabulIn_df['col'] == col_idx + 1))]
    same_row = tabulIn_df[(tabulIn_df['row'] == row_idx) & ((tabulIn_df['col'] == col_idx - 1) | (tabulIn_df['col'] == col_idx + 1))]
    
    vizviv = row_above['val'].sum() + row_below['val'].sum() + same_row['val'].sum()
    
    return vizviv

def get_val(tabulIn_df, row_idx, col_idx):
    # Get the cell value from the DataFrame
    filtered_df = tabulIn_df.filter((col("row") == row_idx) & (col("col") == col_idx))
    if filtered_df.count() == 0:
        return 0
    else:
        return filtered_df.select("val").collect()[0][0]

def correto(tabul, tam):
    # Count the number of live cells in the list
    cnt = sum(val for val in tabul)

    # Check if the conditions for a correct result are met
    return cnt == 5 and tabul[tam - 3] and tabul[tam - 2] and tabul[tam - 1] and tabul[tam - 1] and tabul[tam - 1]


def init_tabul(tam):
    # Initialize the tabulIn with some live cells
    return [
        (1, 2, 1),
        (2, 3, 1),
        (3, 1, 1),
        (3, 2, 1),
        (3, 3, 1)
    ]

# Define the UDF to pass both row and col as separate columns to the update_cell function
update_cell_udf = udf(lambda row, col: update_cell(row, col), IntegerType())

def main():
    POWMIN = 3
    POWMAX = 10

    # Initialize Spark session
    spark = SparkSession.builder.appName("GameOfLife").getOrCreate()

    for pow in range(POWMIN, POWMAX + 1):
        tam = 1 << pow

        # Allocate and initialize tabuleiros
        t0 = wall_time()
        tabulIn = init_tabul(tam)
        tabulIn_df = pd.DataFrame(tabulIn, columns=["row", "col", "val"])  # Initialize tabulIn_df as Pandas DataFrame
        t1 = wall_time()

        for i in range(2 * (tam - 3)):
            # Perform two iterations of life simulation using Pandas DataFrame
            tabulIn_df['val'] = tabulIn_df.apply(lambda row: update_cell(row['row'], row['col'], row['val'], tabulIn_df), axis=1)

        # Convert the updated Pandas DataFrame back to Spark DataFrame
        tabulOut_df = spark.createDataFrame(tabulIn_df)

        # Convert the Spark DataFrame to a Pandas DataFrame and then to a list of integers
        tabulOut_list = tabulOut_df.orderBy("row", "col").toPandas()["val"].tolist()

        if correto(tabulOut_list, tam):
            print("**Ok, RESULTADO CORRETO**")
        else:
            print("**Nok, RESULTADO ERRADO**")

        t2 = wall_time()

        print(f"tam={tam}; tempos: init={t1 - t0:.7f}, comp={t2 - t1:.7f}, tot={t2 - t0:.7f}")

    spark.stop()

if __name__ == "__main__":
    main()