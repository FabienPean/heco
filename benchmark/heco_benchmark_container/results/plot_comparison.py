import pandas as pd
import numpy as np
import json
import matplotlib.pyplot as plt
import os
import hashlib
import glob
import sys


path = os.path.dirname(os.path.realpath(__file__))
case = 'insert'
lib = 'wsl_gcc'

layout_style = {
    'legend_title':'<b>Container</b>',
    'hovermode':"x",
    'dragmode' :'pan',
    'title':{ 
            #'text': 'title',
            'x':0.5,
            'y':1.0,
            'xanchor': 'center',
            'yanchor': 'auto',
            'xref':'paper',
            'yref':'container',
          },
    'margin':dict(l=0, r=0, t=25, b=0),
    'xaxis_title':"#_types",
    'yaxis_title':"cpu_time [ns]",
    #yaxis_type="log",
    'paper_bgcolor':"rgba(0,0,0,0)",
    'plot_bgcolor':'rgba(230,235,245,0.0)',
    'modebar' : {
        #'orientation': 'v',
        'bgcolor': 'rgba(0,0,0,0)',
        'color':'#666',
        'activecolor':'#000'
    },
    'yaxis' : {'hoverformat':'.0f',#change hover display precision
             'showline':True,
             'mirror': True,
             'zeroline':True,
             'showgrid':True,
             #'color':'rgba(0,0,0,0)',
             'linecolor':'#333',
             'gridcolor':'#aaa',
             'zerolinecolor':'#aaa',
             #'linewidth': 1,
             'gridwidth': 0.5,
             },
    'xaxis' : {'showline': True,
             'mirror': True,
             'zeroline': True,
             'showgrid': True,
             #'color':'rgba(0,0,0,0)',
             'linecolor':'#333',
             'gridcolor':'#aaa',
             'zerolinecolor':'#aaa',
             #'linewidth': 1,
             'gridwidth': 0.5,
             },
}



print('Number of arguments:', len(sys.argv), 'arguments.')
print('Argument List:', str(sys.argv))
case = str(sys.argv[1]) if len(sys.argv) == 3 else case #number of types to generate
lib = str(sys.argv[2]) if len(sys.argv) == 3 else lib #number of types to generate


df = pd.DataFrame()
with open(os.path.join(path,'out_'+case+'_'+lib+'.json')) as f:
    data = json.load(f)
df = pd.DataFrame(data['benchmarks'])#keep only benchmark data

df[['run_name','n']] = df['run_name'].str.split('/',expand=True)
df['container']=df['run_name'].str.replace('_'+case,'')#cleanup run names

df_avg=df[df["name"].str.contains("mean")]#keep only the data containing the mean value
df_std=df[df["name"].str.contains("stddev")]#keep only the data containing the std dev value

import plotly.express as px

fig = px.line(df_avg,
            x='n',
            y='cpu_time',
            color='container',
            error_y=df_std['cpu_time'],
            text='container',
            hover_name='container',
            hover_data=['n','cpu_time'],
            log_x=False,
            log_y=False,
            color_discrete_sequence=px.colors.qualitative.D3[1:10],
            line_dash_sequence=['solid','dash']
            )
# set visibility property by name of trace
if 'insert' in case:
    for trace in fig['data']: 
        if(trace['name'] == 'entt_reg_1by1'): trace['visible'] = 'legendonly'
if 'get' in case:
    for trace in fig['data']: 
        if(trace['name'] == 'vecany'): trace['visible'] = 'legendonly'

fig.update_traces(mode="markers+lines", hovertemplate='%{text}:%{y:.01f}<extra></extra>')#change hover display precision
fig.update_layout(**layout_style)
fig.update_layout(
    title={ 
        'text': case,
    },
)
#fig.update_traces(name=df_mean.columns.values, showlegend = True)
fig.show()
filename_output=os.path.join(path,case+'_'+lib)
fig.write_html(filename_output+".html",include_plotlyjs='cdn',full_html=True)

#filename_output=os.path.join(path,os.path.splitext(filename)[0]+'_post')
#df.to_json(path_or_buf=filename_output+'.json', orient="records")
#df.to_csv(path_or_buf=filename_output+'.csv')
#df.to_excel(path_or_buf=filename_output+'.xls')




