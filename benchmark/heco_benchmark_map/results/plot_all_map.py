import pandas as pd
import numpy as np
import json
import matplotlib.pyplot as plt
import os
import hashlib
import glob


layout_style = {
    'legend_title':'<b>Implementation</b>',
    'hovermode':"x",
    'dragmode' :'pan',
    'title': { 
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
    'plot_bgcolor':'rgba(230,235,245,0.0)',#'#ccd2dd'
    'modebar' : {
        #'orientation': 'v',
        'bgcolor': 'rgba(0,0,0,0)',
        'color':'#666',
        'activecolor':'#000'
    },
    'yaxis' : {
        'hoverformat':'.0f',#change hover display precision
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
    'xaxis' : {
        'showline': True,
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


path = os.path.dirname(os.path.realpath(__file__))
suffix = '_win_clang'
df = pd.DataFrame()
for (i,file) in enumerate(list(glob.glob(os.path.join(path,'*'+suffix+'.json')))): 
    with open(os.path.join(path,file)) as f:
        data = json.load(f)
    tmp_df = pd.DataFrame(data['benchmarks'])#keep only benchmark data
    tmp_df['map_from'] = tmp_df['run_name'].str.split('_')
    tmp_df['map_from'] = tmp_df['map_from'].apply(lambda x:x[-1])
    df = df.append(tmp_df, ignore_index=True, sort=False)

df['map_from']=df['map_from'].str.replace('abseil','abseil::flat_hash_map')
df['map_from']=df['map_from'].str.replace('btree','abseil::btree_map')
df['map_from']=df['map_from'].str.replace('boost','boost::flat_map')
df['map_from']=df['map_from'].str.replace('unordered','std::unordered_map')
df['map_from']=df['map_from'].str.replace('hopscotch','tsl::hopscotch')
df['map_from']=df['map_from'].str.replace('robin','tsl::robin_map')
df['map_from']=df['map_from'].str.replace('sparse','tsl::sparse_map')
df['map_from']=df['map_from'].str.replace('spp','spp::sparsepp')

df[['run_name','n']] = df['run_name'].str.split('/',expand=True)

df['n'] = df['n'].str.split('_').apply(lambda x:x[0])
tmp = pd.DataFrame()
tmp['n']=df['n']
tmp['cpu_time']=0.266+((1.27-0.266)/8*(df['n'].apply(pd.to_numeric)-1))#store type_id generation time. For 1 type: 0.266ns, for 9 types: 1.27ns

df_get=df[df["name"].str.contains("get")]#retrieve std deviation data in a new data frame
#df_get[['run_name','n']] = df_get['run_name'].str.split('/',expand=True)
df_get_avg=df_get[df_get["name"].str.contains("mean")]#keep only the data containing the mean value
df_get_std=df_get[df_get["name"].str.contains("stddev")]#keep only the data containing the std dev value

df_insert=df[df["name"].str.contains("insert")]#retrieve std deviation data in a new data frame
#df_insert[['run_name','n']] = df_insert['run_name'].str.split('/',expand=True)
df_insert_avg=df_insert[df_insert["name"].str.contains("mean")]#keep only the data containing the mean value
df_insert_std=df_insert[df_insert["name"].str.contains("stddev")]#keep only the data containing the std dev value

import plotly.express as px
import plotly.graph_objs as go
fig = px.line(df_get_avg,
            x='n',
            y='cpu_time',
            color='map_from',
            error_y=df_get_std['cpu_time'],
            #text='map_from',
            #hover_name='map_from',
            hover_data=['n','cpu_time'],
            log_x=False,
            log_y=False,
            )
fig.update_traces(mode="markers+lines", hovertemplate=None)#'%{text}:%{y:.01f}<extra></extra>')#change hover display precision
fig.update_layout(**layout_style)
fig.update_layout(
    #legend={'tracegroupgap':10},
    title = { 
        'text': 'get',
    },
    yaxis = {
        'hoverformat':'.01f',
        'rangemode':'nonnegative'
    },
)

#fig.update_xaxes(showgrid=True, gridwidth=1, gridcolor='#ddd')
#fig.update_yaxes(showgrid=True, gridwidth=1, gridcolor='#ddd')

#fig.update_traces(legendgroup='group')
#fig.add_trace(go.Scatter(x=tmp['n'], y=tmp['cpu_time'],name='type_id',legendgroup='other'))

fig.show()
filename_output=os.path.join(path,'comparison_map_access'+suffix)
fig.write_html(filename_output+".html",include_plotlyjs='cdn',full_html=True)


fig = px.line(df_insert_avg,
            x='n',
            y='cpu_time',
            color='map_from',
            error_y=df_insert_std['cpu_time'],
            #labels={'map_from':'abseil::flat_hash_map','map_from':'boost::flat_hash_map','map_from':'std::unordered_map'},
            #text='map_from',
            #hover_name='map_from',
            hover_data=['cpu_time'],
            custom_data=[df_insert_std['cpu_time']],
            log_x=False,
            log_y=False
            )
fig.update_traces(mode="markers+lines", hovertemplate=None)#'%{y:.0f}±%{customdata[0]:.0f}<extra>%{name}</extra>')#change hover display precision
fig.update_layout(layout_style)
fig.update_layout(
    title = {
        'text':'insert'
    },
    yaxis = {
        'hoverformat':'.0f',#change hover display precision
    },
)
fig.show()
filename_output=os.path.join(path,'comparison_map_insertion'+suffix)
fig.write_html(filename_output+".html",include_plotlyjs='cdn',full_html=True)
