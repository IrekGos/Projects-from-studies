import argparse
import json
import psycopg2
from sys import exit


def sort(tab):
    for i in range(len(tab)-1):
        for j in range(len(tab)-i-1):
            if (tab[j]["takeoff_time"] < tab[j+1]["takeoff_time"]) or (tab[j]["takeoff_time"] == tab[j+1]["takeoff_time"] and tab[j]["rid"] > tab[j]["rid"]):
                tab[j], tab[j+1] = tab[j+1], tab[j]


def PrettyPrint(data):
    print('{\n"status":"OK",')
    print('"data":%s' % data)
    print("}")


def InitializeDatabase(cur):
    with open("model_fizyczny.sql", "r") as initial_database:
        cur.execute(initial_database.read())
    print('{"status": "OK"}')


def Flight(id, airports, cursor):
    for airport in airports:
        if ("takeoff_time" in airport and "landing_time" in airport):
            cursor.execute("INSERT INTO flights VALUES (%s, %s, %s, %s)", (
                id, airport["airport"], airport["takeoff_time"], airport["landing_time"]))
        elif ("takeoff_time" in airport):
            cursor.execute("INSERT INTO flights VALUES (%s, %s, %s, NULL)",
                           (id, airport["airport"], airport["takeoff_time"]))
        elif ("landing_time" in airport):
            cursor.execute("INSERT INTO flights VALUES (%s, %s, NULL, %s)",
                           (id, airport["airport"], airport["landing_time"]))
    print('{"status": "OK"}')


def ListFlights(id, cursor):
    cursor.execute("SELECT * FROM flights WHERE id=%s;", [id])
    flights = cursor.fetchall()
    cursor.execute("SELECT * FROM flights WHERE id!=%s;", [id])
    other_flights = cursor.fetchall()
    airports = []
    for f in flights:
        cursor.execute(
            "SELECT longitude, latitude FROM airport WHERE iatacode=%s", [f[1]])
        airports.append(cursor.fetchone())
    coordinates = []
    for f in other_flights:
        cursor.execute(
            "SELECT longitude, latitude FROM airport WHERE iatacode=%s", [f[1]])
        coordinates.append(cursor.fetchone())
    result = []
    for i in range(len(flights)-1):
        for j in range(len(other_flights)-1):
            if other_flights[j][2] != None:  # ostatni segment lotu
                cursor.execute("SELECT ST_Distance('LINESTRING(%s %s, %s %s)'::geography, 'LINESTRING(%s %s, %s %s)'::geography)/1000",
                               (airports[i][0], airports[i][1], airports[i+1][0], airports[i+1][1],
                                coordinates[j][0], coordinates[j][1], coordinates[j+1][0], coordinates[j+1][1]))
                distance = cursor.fetchone()
                if(distance[0] == 0.0):
                    entry = {"rid": other_flights[j][0], "from": other_flights[j][1],
                             "to": other_flights[j+1][1], "takeoff_time": str(other_flights[j][2])}
                    if entry not in result:
                        result.append(entry)
    sort(result)
    res_json = json.dumps(result)
    PrettyPrint(res_json)


def ListAirport(iatacode, n, cursor):
    cursor.execute(
        "SELECT id FROM flights WHERE takeoff_time IS NOT NULL AND iatacode=%s ORDER BY takeoff_time DESC, id LIMIT %s;", (iatacode, n))
    raw_result = cursor.fetchall()
    result = []
    for r in raw_result:
        result.append({"id": r[0]})
    res_json = json.dumps(result)
    PrettyPrint(res_json)


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument('--init', action='store_true')

    conn = psycopg2.connect(
        "dbname=student user=app host=localhost password=qwerty")
    conn.autocommit = True
    cur = conn.cursor()

    args = parser.parse_args()
    if(args.init):
        InitializeDatabase(cur)
        conn.close()
        cur.close()
        exit()
    else:
        query = input()
        while query != "quit":
            try:
                json_query = json.loads(query)
            except:
                print('{"status":"ERROR"}')
                break
            if json_query["function"] == "flight":
                Flight(json_query["params"]["id"],
                       json_query["params"]["airports"], cur)
            elif json_query["function"] == "list_flights":
                ListFlights(json_query["params"]["id"], cur)
            elif json_query["function"] == "list_airport":
                ListAirport(json_query["params"]["iatacode"],
                            json_query["params"]["n"], cur)
            else:
                print('{"status":"ERROR"}')
                break
            query = input()
        conn.close()
        cur.close()
        exit()
