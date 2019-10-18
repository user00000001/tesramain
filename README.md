TESRA Core integration/staging repository
=====================================

[![Build Status](https://travis-ci.org/TESRA-Project/TESRA.svg?branch=master)](https://travis-ci.org/TESRA-Project/TESRA) [![GitHub version](https://badge.fury.io/gh/TESRA-Project%2FTESRA.svg)](https://badge.fury.io/gh/TESRA-Project%2FTESRA)

TESRA is a cutting edge cryptocurrency, with many features not available in most other cryptocurrencies.
- Anonymized transactions using coin mixing technology, we call it _Obfuscation_.
- Fast transactions featuring guaranteed zero confirmation transactions, we call it _SwiftTX_.
- Decentralized blockchain voting providing for consensus based advancement of the current Masternode
  technology used to secure the network and provide the above features, each Masternode is secured
  with a collateral of 10K TSR.

More information at [tesra.io](http://www.tesra.io) Visit our ANN thread at [BitcoinTalk](http://www.bitcointalk.org/index.php?topic=5080811)

### Download
#### Ubuntu 18.04
https://github.com/TESRA-Project/TESRA/releases/download/v2.0.0/tesra-qml-2.0.0-ubuntu18.04.zip
#### Windows 7,10 (64-bit)
https://github.com/TESRA-Project/TESRA/releases/download/v2.0.0/tesra-qml-2.0.0-win.zip
#### Android 4.4+
https://github.com/TESRA-Project/TESRA/releases/download/v2.0.0/app-release-1.0.apk
#### IOS
https://itunes.apple.com/cn/app/tesrawallet/id1441621155?l=zh&ls=1&mt=8


### Coin Specs
<table>
<tr><td>Algo</td><td>scrypt</td></tr>
<tr><td>Block Time</td><td>60 Seconds</td></tr>
<tr><td>Difficulty Retargeting</td><td>Every Block</td></tr>
<tr><td>Max Coin Supply (PoW Phase)</td><td>10,000,014,390 TSR</td></tr>
<tr><td>Max Coin Supply (PoS Phase)</td><td>Infinite</td></tr>
<tr><td>Premine</td><td>10,000,000,000 TSR</td></tr>
</table>

### Reward Distribution

<table>
<th colspan=4>Genesis Block</th>
<tr><th>Block Height</th><th>Reward Amount</th><th>Notes</th></tr>
<tr><td>1-400000</td><td>3,452,400,000 TSR</td><td>Pow Rewards</td></tr>
</table>

### PoW Rewards Breakdown

<table>
<th>Block Height</th><th>Reward</th><th>Masternodes</th><th>Miner</th><th>Budget</th>
<tr><td>1-80000</td><td>21920 TSR</td><td>50% (10960 TSR)</td><td>50% (10960 TSR)</td><td>N/A</td></tr>
<tr><td>80001-160000</td><td>10960 TSR</td><td>50% (5480 TSR)</td><td>50% (5480 TSR)</td><td>N/A</td></tr>
<tr><td>160001-240000</td><td>5480 TSR</td><td>50% (2740 TSR)</td><td>50% (2740 TSR)</td><td>N/A</td></tr>
<tr><td>240001-320000</td><td>2740 TSR</td><td>50% (1370 TSR)</td><td>50% (1370 TSR)</td><td>N/A</td></tr>
<tr><td>320001-400000</td><td>1370 TSR</td><td>50% (685 TSR)</td><td>50% (685 TSR)</td><td>N/A</td></tr>
</table>

### PoS + PoW Rewards Breakdown

<table>
<th>Phase</th><th>Block Height</th><th>Reward</th><th>Masternodes & Stakers</th><th>Miner</th><th>Budget</th>
<tr><td>Phase 0</td><td>400001-1976800</td><td>685 TSR</td><td>60% (411 TSR)</td><td>40% (274 TSR)</td><td>N/A</td></tr>
<tr><td>Phase 1</td><td>1976801-3553600</td><td>570.83333333 TSR</td><td>60% (342.49999999 TSR)</td><td>40% (228.33333334 TSR)</td><td>N/A</td></tr>
<tr><td>Phase 2</td><td>3553601-5130400</td><td>475.69444444 TSR</td><td>60% (285.41666666 TSR)</td><td>40% (190.27777778 TSR)</td><td>N/A</td></tr>
<tr><td>Phase 3</td><td>5130401-6707200</td><td>396.41203703 TSR</td><td>60% (237.84722222 TSR)</td><td>40% (158.56481481 TSR)</td><td>N/A</td></tr>
<tr><td>Phase 4</td><td>6707201-8284000</td><td>330.34336419 TSR</td><td>60% (198.20601851 TSR)</td><td>40% (132.13734568 TSR)</td><td>N/A</td></tr>
<tr><td>Phase 5</td><td>8284001-9860800</td><td>275.28613683 TSR</td><td>60% (165.17168209 TSR)</td><td>40% (110.11445474 TSR)</td><td>N/A</td></tr>
<tr><td>Phase 6</td><td>9860801-11437600</td><td>229.40511402 TSR</td><td>60% (137.64306841 TSR)</td><td>40% (91.76204561 TSR)</td><td>N/A</td></tr>
<tr><td>Phase 7</td><td>11437601-13014400</td><td>191.17092835 TSR</td><td>60% (114.70255701 TSR)</td><td>40% (76.46837134 TSR)</td><td>N/A</td></tr>
<tr><td>Phase 8</td><td>13014401-14591200</td><td>159.30910696 TSR</td><td>60% (95.58546417 TSR)</td><td>40% (63.72364279 TSR)</td><td>N/A</td></tr>
<tr><td>Phase 9</td><td>14591201-16168000</td><td>132.75758913 TSR</td><td>60% (79.65455347 TSR)</td><td>40% (53.10303566 TSR)</td><td>N/A</td></tr>
<tr><td>Phase 10</td><td>16168001-17744800</td><td>110.63132427 TSR</td><td>60% (66.37879456 TSR)</td><td>40% (44.25252971 TSR)</td><td>N/A</td></tr>
<tr><td>Phase 11</td><td>17744801-19321600</td><td>92.19277023 TSR</td><td>60% (55.31566213 TSR)</td><td>40% (36.87710810 TSR)</td><td>N/A</td></tr>
<tr><td>Phase 12</td><td>19321601-20898400</td><td>76.82730852 TSR</td><td>60% (46.09638511 TSR)</td><td>40% (30.73092341 TSR)</td><td>N/A</td></tr>
<tr><td>Phase 13</td><td>20898401-22475200</td><td>64.02275710 TSR</td><td>60% (38.41365426 TSR)</td><td>40% (25.60910284 TSR)</td><td>N/A</td></tr>
<tr><td>Phase 14</td><td>22475201-24052000</td><td>53.35229758 TSR</td><td>60% (32.01137854 TSR)</td><td>40% (21.34091904 TSR)</td><td>N/A</td></tr>
<tr><td>Phase 15</td><td>24052001-25628800</td><td>44.46024799 TSR</td><td>60% (26.67614879 TSR)</td><td>40% (17.78409920 TSR)</td><td>N/A</td></tr>
<tr><td>Phase 16</td><td>25628801-27205600</td><td>37.05020665 TSR</td><td>60% (22.23012399 TSR)</td><td>40% (14.82008266 TSR)</td><td>N/A</td></tr>
<tr><td>Phase 17</td><td>27205601-28782400</td><td>30.87517221 TSR</td><td>60% (18.52510332 TSR)</td><td>40% (12.35006889 TSR)</td><td>N/A</td></tr>
<tr><td>Phase 18</td><td>28782401-30359200</td><td>25.72931017 TSR</td><td>60% (15.43758610 TSR)</td><td>40% (10.29172407 TSR)</td><td>N/A</td></tr>
<tr><td>Phase 19</td><td>30359201-31936000</td><td>21.44109181 TSR</td><td>60% (12.86465508 TSR)</td><td>40% (8.57643673 TSR)</td><td>N/A</td></tr>
<tr><td>Phase 20</td><td>31936001-33512800</td><td>17.86757651 TSR</td><td>60% (10.72054590 TSR)</td><td>40% (7.14703061 TSR)</td><td>N/A</td></tr>
<tr><td>Phase 21</td><td>33512801-35089600</td><td>14.88964709 TSR</td><td>60% (8.93378825 TSR)</td><td>40% (5.95585884 TSR)</td><td>N/A</td></tr>
<tr><td>Phase 22</td><td>35089601-36666400</td><td>12.40803924 TSR</td><td>60% (7.44482354 TSR)</td><td>40% (4.96321570 TSR)</td><td>N/A</td></tr>
<tr><td>Phase 23</td><td>36666401-38243200</td><td>10.34003270 TSR</td><td>60% (6.20401962 TSR)</td><td>40% (4.13601308 TSR)</td><td>N/A</td></tr>
<tr><td>Phase 24</td><td>38243201-39820000</td><td>8.61669392 TSR</td><td>60% (5.17001635 TSR)</td><td>40% (3.44667757 TSR)</td><td>N/A</td></tr>
<tr><td>Phase 25</td><td>39820001-41396800</td><td>7.18057826 TSR</td><td>60% (4.30834695 TSR)</td><td>40% (2.87223131 TSR)</td><td>N/A</td></tr>
<tr><td>Phase 26</td><td>41396801-42973600</td><td>5.98381522 TSR</td><td>60% (3.59028913 TSR)</td><td>40% (2.39352609 TSR)</td><td>N/A</td></tr>
<tr><td>Phase 27</td><td>42973601-44550400</td><td>4.98651268 TSR</td><td>60% (2.99190760 TSR)</td><td>40% (1.99460508 TSR)</td><td>N/A</td></tr>
<tr><td>Phase 28</td><td>44550401-46127200</td><td>4.15542723 TSR</td><td>60% (2.49325633 TSR)</td><td>40% (1.66217090 TSR)</td><td>N/A</td></tr>
<tr><td>Phase 29</td><td>46127201-47704000</td><td>3.46285603 TSR</td><td>60% (2.07771361 TSR)</td><td>40% (1.38514242 TSR)</td><td>N/A</td></tr>
<tr><td>Phase 30</td><td>47704001-49280800</td><td>2.88571335 TSR</td><td>60% (1.73142801 TSR)</td><td>40% (1.15428534 TSR)</td><td>N/A</td></tr>
<tr><td>Phase 31</td><td>49280801-50857600</td><td>2.40476113 TSR</td><td>60% (1.44285667 TSR)</td><td>40% (0.96190446 TSR)</td><td>N/A</td></tr>
<tr><td>Phase 32</td><td>50857601-52434400</td><td>2.00396761 TSR</td><td>60% (1.20238056 TSR)</td><td>40% (0.80158705 TSR)</td><td>N/A</td></tr>
<tr><td>Phase 33</td><td>52434401-54011200</td><td>1.66997300 TSR</td><td>60% (1.00198380 TSR)</td><td>40% (0.66798920 TSR)</td><td>N/A</td></tr>
<tr><td>Phase 34</td><td>54011201-55588000</td><td>1.39164417 TSR</td><td>60% (0.83498650 TSR)</td><td>40% (0.55665767 TSR)</td><td>N/A</td></tr>
<tr><td>Phase 35</td><td>55588001-57164800</td><td>1.15970347 TSR</td><td>60% (0.69582208 TSR)</td><td>40% (0.46388139 TSR)</td><td>N/A</td></tr>
<tr><td>Phase X</td><td>57164801-Infinite</td><td>0 TSR</td><td>60% (0 TSR)</td><td>40% (0 TSR)</td><td>N/A</td></tr>
</table>
