import plotly
import plotly.plotly as py
import plotly.graph_objs as go
import plotly.figure_factory as FF
import plotly.io as pio
plotly.tools.set_credentials_file(username='jt7991', api_key='AgB4mwyCuvY2xPGYy1ei')
import numpy as np
import pandas as pd

df72 = pd.read_csv('data72_2.csv')
df74 = pd.read_csv('data74_2.csv')
df76 = pd.read_csv('data76_2.csv')

arrivalsData = [go.Scatter(
    x=df72['timeStep'],
    y=df72[' arrived']
    )]
arrivalsLayout = go.Layout(
    title='People Arrival',
    yaxis=dict(title='Arrived'),
    xaxis=dict(title='Time (minutes)')
)
arrivalsFig = go.Figure(data=arrivalsData, layout=arrivalsLayout)

waitingData = [
    go.Scatter(
        x=df72['timeStep'],
        y=df72[' waitingInLine'],
        name="MAXPERCAR = 7, CARNUM = 2"
    ),
    go.Scatter(
        x=df74['timeStep'],
        y=df74[' waitingInLine'],
        name="MAXPERCAR = 7, CARNUM = 4"
    ),
    go.Scatter(
        x=df76['timeStep'],
        y=df76[' waitingInLine'],
        name="MAXPERCAR = 7, CARNUM = 6"
    )
]
waitingLayout = go.Layout(
    title='Waiting Queue',
    yaxis=dict(
        title='# of People Waiting',
        range=[0,1000],
        dtick=100,
        showgrid=True,
        ),
    xaxis=dict(
        title='Time (minutes)',
        dtick=100,
        range=[0,600],
        showgrid=True
        ),
        legend=dict(orientation="h",x=1, y=1)
)
waitingFig = go.Figure(data=waitingData, layout=waitingLayout)

leavingData = [
    go.Scatter(
        x=df72['timeStep'],
        y=df72[' rejected'],
        name="MAXPERCAR = 7, CARNUM = 2"
    ),
    go.Scatter(
        x=df74['timeStep'],
        y=df74[' rejected'],
        name="MAXPERCAR = 7, CARNUM = 4"
    ),
    go.Scatter(
        x=df76['timeStep'],
        y=df76[' rejected'],
        name="MAXPERCAR = 7, CARNUM = 6"
    )
]
leavingLayout = go.Layout(
    title='People Sent Away',
    yaxis=dict(
        title='# of People Leaving',
        autorange=True,
        dtick=5,
        showgrid=True,
        ),
    xaxis=dict(
        title='Time (minutes)',
        dtick=100,
        range=[0,600],
        showgrid=True
        ),
        legend=dict(orientation="h",x=1, y=1)
)
leavingFig = go.Figure(data=leavingData, layout=leavingLayout)
pio.write_image(arrivalsFig,'arrivals.png')
pio.write_image(waitingFig,'waiting.png')
pio.write_image(leavingFig,'leaving.png')