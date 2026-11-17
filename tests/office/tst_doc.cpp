// Tests of libs/office/doc: reading an old binary word processing file into the document the .docx reader makes. Every file is built in
// memory by dochelpers.h; the hostile ones are that file changed.
#include "dochelpers.h"
#include "doc/doccrypt.h"
#include "office/package.h"

#include "doc/doc.h"
#include "doc/docsalvage.h"
#include "office/cfbsalvage.h"
#include "io/cfb.h"
#include "wml/wml.h"

#include <QBuffer>
#include <QDir>
#include <QDirIterator>
#include <QImage>
#include <QImageReader>
#include <QElapsedTimer>
#include <QRandomGenerator>
#include <QXmlStreamReader>
#include <QtTest>

#include "../testrunner.h"

using namespace dochelp;
using namespace jp::office;
using jp::office::wml::Block;
using jp::office::wml::Document;
using jp::office::wml::Inline;
using jp::office::wml::ReadNotes;
using jp::office::Package;

namespace {

// The known-answer streams: made by tools/doc_encrypt_kat.py from the published rules alone, never by the program's own code (password "Pa\xc3\x9fw\xc3\xb6rd1").
const char *const kDocKatRC440_wd =
    "7KURGB8mLTQ7QgADV140AAAAgYiPlp2kq7K5wMfO1dzj6vH4/wYNFBsiKTA3PkVMU1phaG92fYSLkpmgp661vMPK0dhTG1hSjTLX"
    "ARxFZqP/YAlNqehZgQueIokPF6kj8GuT+xILTwMSYiHrsGjDyGLpoWg14bTm+EDjnEPzD4O6qbgMMSQri1ufOEFVpuKfcXddusuj"
    "a/e+c/EqJRqhTDJFfVfi+BuAtvlEFKXAADR//wGVMii73DZLz4gaDQij3MZHnM2uMY3D+xTarYuK63qeIqljjfhaDob/Xoz5HF4Y"
    "r6eiikjvV9aoOLid6PoiZFrsGd18oRrC4NEwEkWMyp0Gzc9JFtR1RsYZ2G39+g+ZrlS/inEdolnmkS6Pyf2h/XPIjzABPhqoqS2x"
    "dn+zAGDYph+d6YAY0qMWcUfLpGYW+i3aHafAWp5If3aeANXgLCtWl4Sff24H8rB53M8yj1g17OYJyS61ObLt8At5YSofVFvw9MZu"
    "FlrpINucT+d5VIzEAdeZ/RnIKE/jnaRMvw4NipJOxKvE4L3hK2wUk/dqDbpFQFUJb/8vy0cdrwBbuB8z/FcMS40DygvzjJjEqWGV"
    "tPNfmepxgcsjUAe7DKKH8pWp63+HA3L2P1CcC8b0j33/Df+Ga6vNFk8eo4/XpyG2LurK3bQmeTKGdd40yFq6Fz2wr4L4XF3IamrX"
    "f/akhg76nSjpOOUWW9GbiaUdMrYs86bIrPK10V1TiDT6H4Arce09VE/j+fId/LuniTjvd+lupcJMiHpYDvE3vQhRrAri8CPGN43n"
    "WNrzfKIz2Z4Wfx+OmNNvleMlCJhI+4Sep9oopaYh9KnsqtPOLBAU0H0PMN3yF/li3TcT+mAjkJTU+pdYysfEynUe9CSYS4+bXrvD"
    "uFcFyijt1k9hzEFWS4paKA+zkzYPw6kbuFa38LEEDVO/ni4VCGnMPZ8vkwMqWwegkihWt4XV0bFMsNvR5xrZMD0jBvuJ9qhecPQn"
    "KA55SJILnAV21JH7vLjfSt3y/jdXJttq7LE7mDXNHr+KuvkTPAWYvRYW6IWRZqfaOK5LHe3g0jDLG4xb/1hX2thOsqCh50aDIakI"
    "T3qz8V96upaxoVzU2q5ED3H+h/bgjC6JDYL2BqM27DCa8/3Fq+9Cu73lWzEw0CLXaF0OvpxnhTfShdoHN0COzQiacQDHrTLZ8xRm"
    "p6aeSrkXRIzvpfYpq11RK7Lkm8AKnl9AUD3rMQN5efYyXBcYU+C3k5oSx7VCFQy5n235JvIsU5SObRR9jZB0IOPHobo2BApJQi1V"
    "ONvjb/5eLIcOCUs5T4fbZWxOdnOCf2ILffVr5dhZYAR2o52FScf02drifLGNwFvHBYcHiRPGXYtANfLu/P8aIH7ltmbY8Zzqb+yR"
    "14B5hwWIncW+WZ0dn4f33aTGSbYv/eFiBOu7dI/VvWK34gd8DCMDCpAYz0/5tf0sLEM=";
const char *const kDocKatRC440_table =
    "AQABAAUSHyw5RlNgbXqHlKGuu8jvZ1qDD7TA9Rm0OjxkQvAIL9g8CdsYsiVFgK8BPWfZjhOB+vdb+02vSzruKpCNKvr/hzQu+aZD"
    "taDp+k8LlJ3ZBVT1HZ/qVh2TuxWPZP9nD/6X47+m9rWfzARfZNZdNfzZHVh6bPRXCN+fc+8uPQy4nbjHd68LrPXpCn7zBQPJLmcf"
    "x2sqhwW+ubYd4KbRCSOOZLc8Am3Q4FksnJjLS5UBXlTXQKL/exyG4fRPSFLzKGEyXfG3b4BuEScWB45qtj3PMVTGmvKLyhBVoPKM"
    "O1NWZtRD62I8rMzhhGaO0O54jTGATYZWVGWkjinwpgmSeXvlijiJslKNbNFRZmPt2sArJs2xPs0SZboTZUENaee8+1ydkqYcPblF"
    "ipMvrNRsMovxleyERhei5dsnWIqCbplusTusJurc68IirEkM2N/CAygj0/KThsTtQGOOI8yhGBLlVYIJjSZ5hHcV/Yar4M9kGDqC"
    "is5dlE8AI5sVwBhwtXsFEeU8vNtXIQjQ03p5HgbieAdYdEkVv/C4L1v+mc4xLMml00u7X7PhQ5z3DKunaDtwJxGXfr9nEHQ4RfUB"
    "AEfzBYYN9V+35LsXkE5zBgE9R8Mrn+aCS8QAp3pYG+kL+RMaxxd5gttq3+NLC5UCun4mIVi67YYy4UJYtDYug4kEAx4UoKlc/t5r"
    "02rI8npuCYRVlHmCryUPFQmhniK4h9KkMF4JZcnHfMgWgyyfxXmpODOPZWapSC87ZcQD433aEW7Q5AYsmmWDAaTNQP4WZLdqiyF7"
    "zK6H6D6fZTKC6+t6dE/DKVexnOw0lxgyE268MUrdGDV4HmdasHxovOmbhGleixWeKaOHTtyPDPigjgPMZntoVuHqALAE5zM3yi+3"
    "zDuZZpRZQtuVMK3K5z7uvJvf71qTVx2vLMpbDF2QmecLMrJ5dB1YqSuTP5/Gr/M0BoTqGxlBpcXYLHdtS45NxMnPmlc1QjzKBIhL"
    "tKLN/AafcPmaSAVPCCxDJqGeaqPjknf2AE3PDKF5ohMW1o1nqJE0AbqKfHFl8jt2hALfiZmUvqxnpzjPa6yrNkTiBhQ1cyr/TTWc"
    "+84nbbOGVgIlFeh4RsI4e+VqM0pMEMJ9+RZiqh+acKTuh2lZB1PuLykRr92sfJ5j/Ml6wvD7KYNmEU7ry6wSWbwu5Zyr0V5NZ6DS"
    "Czpytk2D0DhTCWpF3ynFvx5YN1yeaqvUzJFXnZftDYJewLuk53QjZ2b+Wxn2oZgt8xGVug==";
const char *const kDocKatRC440_data =
    "50UO+W8OVBfR5l4mVJg02pbYppaWbz/89uaALk40mMhnH7bzVT0J0qCzetC6AFzeMz7lOH/9lmvPT/k719YSxgQZnk5TG1hSjTLX"
    "ARxFZqP/YAlNqehZgQueIokPF6kj8GuT+xILTwMSYiHrsGjDyGLpoWg14bTm+EDjnEPzD4O6qbgMMSQri1ufOEFVpuKfcXddusuj"
    "a/e+c/EqJRqhTDJFfVfi+BuAtvlEFKXAADR//wGVMii73DZLz4gaDQij3MZHnM2uMY3D+xTarYuK63qeIqljjfhaDob/Xoz5HF4Y"
    "r6eiikjvV9aoOLid6PoiZFrsGd18oRrC4NEwEkWMyp0Gzc9JFtR1RsYZ2G39+g+ZrlS/inEdolnmkS6Pyf2h/XPIjzABPhqoqS2x"
    "dn+zAGDYph+d6YAY0qMWcUfLpGYW+i3aHafAWp5If3aeANXgLCtWl4Sff24H8rB53M8yj1g17OYJyS61ObLt8At5YSofVFvw9MZu"
    "FlrpINucT+d5VIzEAdeZ/RnIKE/jnaRMvw4NipJOxKvE4L3hK2wUk/dqDbpFQFUJb/8vy0cdrwBbuB8z/FcMS40DygvzjJjEqWGV"
    "tPNfmepxgcsjUAe7DKKH8pWp63+HA3L2P1CcC8b0j33/Df+Ga6vNFk8eo4/XpyG2LurK3bQmeTKGdd40yFq6Fz2wr4L4XF3IamrX"
    "f/akhg76nSjpOOUWW9GbiaUdMrYs86bIrPK10V1TiDT6H4Arce09VE/j+fId/LuniTjvd+lupcJMiHpYDvE3vQhRrAri8CPGN43n"
    "WNrzfKIz2Z4Wfx+OmNNvleMlCJhI+4Sep9oopaYh9KnsqtPOLBAU0H0PMN3yF/li3TcT+mAjkJTU+pdYysfEynUe9CSYS4+bXrvD"
    "uFcFyijt1k9hzEFWS4paKA+zkzYPw6kbuA==";
const char *const kDocKatCRYPTOAPI_wd =
    "7KURGB8mLTQ7QgADV17GAAAAgYiPlp2kq7K5wMfO1dzj6vH4/wYNFBsiKTA3PkVMU1phaG92fYSLkpmgp661vMPK0difjKw36s1B"
    "Yf2r2h9lFDsgLUpvkarLBY5Ox9k2grtBGpdEy+mnoClftyJOpwPDT+y74qzYhxh3Kh5eJseB1bh09PEc77XGTtwXQFueMkVMp9yK"
    "KOTwF2nkaRFMqAh+/4+uuRDU46ODmU0sfH/+qMks+bcgIam9AmVj/IbKHzbA3+4leNcWRafRFTL/ndRrjkWk6cFFPQDoCYXRAEqL"
    "UuiBVZEljw3DNre5DqP40YP+vHome0gNttQ3xPr3TjRInqQ8sPVAnAGQ1722DbsQKB+gs34yhi3BIJdcWzSYoXjB/lJhUkad/zC4"
    "13rfV6PEKjTHUwMztbDoKwBw1MnfZqhF3ICaB7P3PMSZmuTYDXS8O7OqYn1SDmKJ2581tDVBI+t34X58YjHnBX/UpDfZUvTWDXUF"
    "+R80rgGlrPICTCDa+uP+9gqLwh/0J75ovAsPAe4Yv58Gsj402BmhMiqFs9GxVZ64Xm71WN7a4mqvA6Wsl1vLUwWJuHFAzFMi7MY3"
    "9SPYmAAxW86G5kbTjnPLddvKCwX1DmDq/im867bWpMD8/BTzTivaGoDLZeHxgeT3s95nY6Jla8Z0B6iHXUuBQXHXgWFcHhM3pTtZ"
    "1Vil9DkBLswqZiIZXLNuvRtAcQMKkV1tVRGbIRnsiYjlQd8uvhAs4YgLgIIAUqKXr2xzrd0fImoUbCHk9W/i3MteCwFvcRtTOs6r"
    "LfRXaF2cPvC8KCsTVHfe+y8/XQyW8cbU4nYzTT560ezotbLCI++AsctDLj2NaGRappFtztLPQb8PJOl15X7VAjahR+Y3j2gKdB+L"
    "gRV718TFDdU26iDombCzs+Ppn0Xu8H72sNzbGfbUtOic2IOjRiqIIWx4GtliIVbBIIw/RwkCMxvqT+NUcJo6m78mqqfiZj9Hef0l"
    "XtubHk7i23HIAHmi1DmHTr41ld/V+ciY1coiSQ1QJiZnVgzOLk9vI6QFBFnHFPKUkDfhk88nzbnnk1PugjEa5RTiU8/Or00FF7Z4"
    "N0P0vx0ggHs6J0qq+5cuKyWNdBDMOau0mKbs8zNh+psdqGi0yBJvkr8WPUzrs2Z0WtVkJwPt837FAAYB/h2i+LTjK3EYinxyCOmC"
    "IVk/B4DOr6d53tAhOCuLZqyNJfGawavQJ7WrAAGC4zAV40vvWo2xA6SOHEWIX7wqNDqLexDq5lzXbVrGYolT1xEFJ4lEF9dIsZF6"
    "H4PkmUiLX3cvoYY8c8E2UpYA1gVhCoCpCVDe6Aa2qBw33eIBRK/O6Y2/WlujSjfUEzswGKKbOGbGSB46rkPaJqyQYVsnG0z6gOce"
    "OwrO1n9Ai1iaoyAktTJjg1ItrmBRb5ZGfj6l6ify/zaCEm7Wtz+tTIk3ClsryPBCOfM=";
const char *const kDocKatCRYPTOAPI_table =
    "BAACAAQAAAB+AAAABAAAAAAAAAABaAAABIAAAIAAAAABAAAAAAAAAAAAAABFAG4AaABhAG4AYwBlAGQAIABDAHIAeQBwAHQAbwBn"
    "AHIAYQBwAGgAaQBjACAAUAByAG8AdgBpAGQAZQByACwAIAB2AGUAcgBzAGkAbwBuACAAMQAuADAALgAwAAAAEAAAAAUSHyw5RlNg"
    "bXqHlKGuu8gA8U6KUgS9RX22jQ/HEBl6FAAAAOTYI1PPxTVMXPJPuY8sjir5Xu3KFql83zFnr5hhcy6BGNMOU2/bV3qenx97vuQR"
    "uB53vwuLMadZoMHDZD1WayloKpDcldanDGKhXpCJIK7iKBKWKhu+dpsGYQcck9VqQok2GcScGMc71gHG8Yo2OxK3iDj7/Pg3ZaZO"
    "LZBB+RluvKKtKW2tHwpevZqaKidFzB7zdhr0edltqnIjMHo2954qrRkQzOM4dBQfQTWLGq+r1R2de9DCyKtxcwW+Oplj+GJA54/r"
    "Z7WOGJc/xoxs1opsTElkGPRhWIlCnRT20nFll3iyBTGYWMTCToMLjIQf2afHPwQW4MRvziggCPQBuQ86ATGxPZsjAsfWVrncAlyd"
    "Q5VyAm5PMVQQUPx5EJ0xn01cob9bkAqQiL8mQQh4PioKCv5p4JVwgBa9H4tvL15dJUiNmUz7wXzCkTLtIyUb68dhK/uy4PmtM43j"
    "f8bLjlOXuGaQyLzzpkX4J7H+35lg5ysHz78li4N6f3IP33GUFIa6i/JlHii65DQNRZKdN3eplMCOAl+Ob/lUZmHA5fuF5435gGA1"
    "R44h/sc2gF4mwt3lvu1wRYWly3rsm1h6WNyl29SAP3JCDwRUuYX+31HpmIsn8oqkTAv7eGhl39F1Tn/jT8R7nNxbsXCtJdak7nX9"
    "93/heXpvl0PAEMp2NwoZJXWD5StwWsRAJkYx5xhOHl4qchnNOEASt9rCsEeM27xXtiaF6ZdoSW181Unq3gDQbUnMMAlczKXRD4dP"
    "wHUhtNh0MYsmntMYYq8dJMBbD3VjT2ICOzTI05vmnIz5OHakuNnFmQqb7qMxgmg+Lpl7+blRpyNJLfl0FMfgD4pM6WVYOSd/eSjS"
    "jfViJffebuGQkfwAYflQQb8bwqpmp0VOcjB6WYnPZPFn3v4uYqzBCFXgy6ZxHdjewEMSXWlzXcRvlpDrBPM8Ug5Vvety9BLool80"
    "i8PR+WpUORHDdE5PQkEd8AY3i29wO11GvR8VrpvolUZ/eeVR8Bcn9V5kgusy9Sq8XkDl5bpQUMpNh6Qo+CPlYbufSfcujUH+Czvk"
    "8XkOD94h5dmxy/xK5Vuc7DiaPPOX4BoHt/pEfnDMwoKZZ0iX0kU0BxMV4O010F2qfaGarhQxoog4ooSsGPlwuMLqC82xsfZUHg3k"
    "zZxUfMHuETLs1Uq+G4zJGcTbVIrPwSzs6KjPkElsVYw0hPQ8SdE35j+BoMFFtprY";
const char *const kDocKatCRYPTOAPI_data =
    "CNMa8DK+Kae15OkV98rdqAnnSALyi8T9olw3RVuR03ghplLCYrZb21Fc2ujCOpFMeBQ4CM714Po1QKYeCz7iUDHItmCfjKw36s1B"
    "Yf2r2h9lFDsgLUpvkarLBY5Ox9k2grtBGpdEy+mnoClftyJOpwPDT+y74qzYhxh3Kh5eJseB1bh09PEc77XGTtwXQFueMkVMp9yK"
    "KOTwF2nkaRFMqAh+/4+uuRDU46ODmU0sfH/+qMks+bcgIam9AmVj/IbKHzbA3+4leNcWRafRFTL/ndRrjkWk6cFFPQDoCYXRAEqL"
    "UuiBVZEljw3DNre5DqP40YP+vHome0gNttQ3xPr3TjRInqQ8sPVAnAGQ1722DbsQKB+gs34yhi3BIJdcWzSYoXjB/lJhUkad/zC4"
    "13rfV6PEKjTHUwMztbDoKwBw1MnfZqhF3ICaB7P3PMSZmuTYDXS8O7OqYn1SDmKJ2581tDVBI+t34X58YjHnBX/UpDfZUvTWDXUF"
    "+R80rgGlrPICTCDa+uP+9gqLwh/0J75ovAsPAe4Yv58Gsj402BmhMiqFs9GxVZ64Xm71WN7a4mqvA6Wsl1vLUwWJuHFAzFMi7MY3"
    "9SPYmAAxW86G5kbTjnPLddvKCwX1DmDq/im867bWpMD8/BTzTivaGoDLZeHxgeT3s95nY6Jla8Z0B6iHXUuBQXHXgWFcHhM3pTtZ"
    "1Vil9DkBLswqZiIZXLNuvRtAcQMKkV1tVRGbIRnsiYjlQd8uvhAs4YgLgIIAUqKXr2xzrd0fImoUbCHk9W/i3MteCwFvcRtTOs6r"
    "LfRXaF2cPvC8KCsTVHfe+y8/XQyW8cbU4nYzTT560ezotbLCI++AsctDLj2NaGRappFtztLPQb8PJOl15X7VAjahR+Y3j2gKdB+L"
    "gRV718TFDdU26iDombCzs+Ppn0Xu8H72sA==";

const char *const kDocKatXORwd =
    "7KURGB8mLTQ7QgCDV15JjvjSgYiPlp2kq7K5wMfO1dzj6vH4/wYNFBsiKTA3PkVMU1phaG92fYSLkpmgp661vMPK0dhP0qAEJPjY"
    "urlYQMKCurX5XcS2Fjb21qirSlbUkKir50PWpAAg5MSmpURExoa+ufVRyLoSMvLStLdWSsiUrK/jR9qoDCzgwKKhQFjamqK98VXM"
    "vh4+/t6ws1JOzIiws/9b3qwIKOzMrq1MXN6epqHtSdCiGjr62ry/XkLAjLS3+1/CsBQ06MiqqUhQ0pKqpelN1KYGJubGuLtaRsSA"
    "uLv3U8a0EDD01La1VFTWlq6p5UHYqgIi4sKkp0Za2IS8v/NXyrgcPPDQsrFQSMqKsq3hRdyuDi7uzqCjQl7cmKCj70vOvBg4/Ny+"
    "vVxMzo62sf1ZwLIKKurKrK9OUtCcpKfrT9KgBCT42Lq5WEDCgrq1+V3EthY29taoq0pW1JCoq+dD1qQAIOTEpqVERMaGvrn1Uci6"
    "EjLy0rS3VkrIlKyv40faqAws4MCioUBY2pqivfFVzL4ePv7esLNSTsyIsLP/W96sCCjszK6tTFzenqah7UnQoho6+tq8v15CwIy0"
    "t/tfwrAUNOjIqqlIUNKSqqXpTdSmBibmxri7WkbEgLi791PGtBAw9NS2tVRU1pauqeVB2KoCIuLCpKdGWtiEvL/zV8q4HDzw0LKx"
    "UEjKirKt4UXcrg4u7s6go0Je3Jigo+9LzrwYOPzcvr1cTM6OtrH9WcCyCirqyqyvTlLQnKSn60/SoAQk+Ni6uVhAwoK6tfldxLYW"
    "NvbWqKtKVtSQqKvnQ9akACDkxKalRETGhr659VHIuhIy8tK0t1ZKyJSsr+NH2qgMLODAoqFAWNqaor3xVcy+Hj7+3rCzUk7MiLCz"
    "/1verAgo7MyurUxc3p6moe1J0KIaOvravL9eQsCMtLf7X8KwFDToyKqpSFDSkqql6U3UpgYm5sa4u1pGxIC4u/dTxrQQMPTUtrVU"
    "VNaWrqnlQdiqAiLiwqSnRlrYhLy/81fKuBw88NCysVBIyoqyreFF3K4OLu7OoKNCXtyYoKPvS868GDj83L69XEzOjrax/VnAsgoq"
    "6sqsr05S0Jykp+tP0qAEJPjYurlYQMKCurX5XcS2Fjb21qirSlbUkKir50PWpAAg5MSmpURExoa+ufVRyLoSMvLStLdWSsiUrK/j"
    "R9qoDCzgwKKhQFjamqK98VXMvh4+/t6ws1JOzIiws/9b3qwIKOzMrq1MXN6epqHtSdCiGjr62ry/XkLAjLS3+1/CsBQ06MiqqUhQ"
    "0pKqpelN1KYGJubGuLtaRsSAuLv3U8a0EDD01La1VFTWlq6p5UHYqgIi4sKkp0Za2IS8v/NXyrgcPPDQsrFQSMqKsq3hRdyuDi7u"
    "zqCjQl7cmKCj70vOvBg4/Ny+vVxMzo62sf1ZwLIKKurKrK9OUtCcpKfrT9KgBCT42Lo=";
const char *const kDocKatXORtable =
    "hLy/81fKuBw88NCysVBIyoqyreFF3K4OLu7OoKNCXtyYoKPvS868GDj83L69XEzOjrax/VnAsgoq6sqsr05S0Jykp+tP0qAEJPjY"
    "urlYQMKCurX5XcS2Fjb21qirSlbUkKir50PWpAAg5MSmpURExoa+ufVRyLoSMvLStLdWSsiUrK/jR9qoDCzgwKKhQFjamqK98VXM"
    "vh4+/t6ws1JOzIiws/9b3qwIKOzMrq1MXN6epqHtSdCiGjr62ry/XkLAjLS3+1/CsBQ06MiqqUhQ0pKqpelN1KYGJubGuLtaRsSA"
    "uLv3U8a0EDD01La1VFTWlq6p5UHYqgIi4sKkp0Za2IS8v/NXyrgcPPDQsrFQSMqKsq3hRdyuDi7uzqCjQl7cmKCj70vOvBg4/Ny+"
    "vVxMzo62sf1ZwLIKKurKrK9OUtCcpKfrT9KgBCT42Lq5WEDCgrq1+V3EthY29taoq0pW1JCoq+dD1qQAIOTEpqVERMaGvrn1Uci6"
    "EjLy0rS3VkrIlKyv40faqAws4MCioUBY2pqivfFVzL4ePv7esLNSTsyIsLP/W96sCCjszK6tTFzenqah7UnQoho6+tq8v15CwIy0"
    "t/tfwrAUNOjIqqlIUNKSqqXpTdSmBibmxri7WkbEgLi791PGtBAw9NS2tVRU1pauqeVB2KoCIuLCpKdGWtiEvL/zV8q4HDzw0LKx"
    "UEjKirKt4UXcrg4u7s6go0Je3Jigo+9LzrwYOPzcvr1cTM6OtrH9WcCyCirqyqyvTlLQnKSn60/SoAQk+Ni6uVhAwoK6tfldxLYW"
    "NvbWqKtKVtSQqKvnQ9akACDkxKalRETGhr659VHIuhIy8tK0t1ZKyJSsr+NH2qgMLODAoqFAWNqaor3xVcy+Hj7+3rCzUk7MiLCz"
    "/1verAgo7MyurUxc3p6moe1J0KIaOvravL9eQsCMtLf7X8KwFDToyKqpSFDSkqql6U3UpgYm5sa4u1pGxIC4u/dTxrQQMPTUtrVU"
    "VNaWrqnlQdiqAiLiwqSnRlrYhLy/81fKuBw88NCysVBIyoqyreFF3K4OLu7OoKNCXtyYoKPvS868GDj83L69XEzOjrax/VnAsgoq"
    "6sqsr05S0Jykp+tP0qAEJPjYurlYQMKCurX5XcS2Fjb21qirSlbUkKir50PWpAAg5MSmpURExoa+ufVRyLoSMvLStLdWSsiUrK/j";
const char *const kDocKatXORdata =
    "hLy/81fKuBw88NCysVBIyoqyreFF3K4OLu7OoKNCXtyYoKPvS868GDj83L69XEzOjrax/VnAsgoq6sqsr05S0Jykp+tP0qAEJPjY"
    "urlYQMKCurX5XcS2Fjb21qirSlbUkKir50PWpAAg5MSmpURExoa+ufVRyLoSMvLStLdWSsiUrK/jR9qoDCzgwKKhQFjamqK98VXM"
    "vh4+/t6ws1JOzIiws/9b3qwIKOzMrq1MXN6epqHtSdCiGjr62ry/XkLAjLS3+1/CsBQ06MiqqUhQ0pKqpelN1KYGJubGuLtaRsSA"
    "uLv3U8a0EDD01La1VFTWlq6p5UHYqgIi4sKkp0Za2IS8v/NXyrgcPPDQsrFQSMqKsq3hRdyuDi7uzqCjQl7cmKCj70vOvBg4/Ny+"
    "vVxMzo62sf1ZwLIKKurKrK9OUtCcpKfrT9KgBCT42Lq5WEDCgrq1+V3EthY29taoq0pW1JCoq+dD1qQAIOTEpqVERMaGvrn1Uci6"
    "EjLy0rS3VkrIlKyv40faqAws4MCioUBY2pqivfFVzL4ePv7esLNSTsyIsLP/W96sCCjszK6tTFzenqah7UnQoho6+tq8v15CwIy0"
    "t/tfwrAUNOjIqqlIUNKSqqXpTdSmBibmxri7WkbEgLi791PGtBAw9NS2tVRU1pauqeVB2KoCIuLCpKdGWtiEvL/zV8q4HDzw0LKx"
    "UEjKirKt4UXcrg4u7s6go0Je3Jigo+9LzrwYOPzcvr1cTM6OtrH9WcCyCirqyqyvTlLQnKSn60/SoAQk+Ni6uVhAwoK6tfldxLYW"
    "NvbWqKtKVtSQqKvnQ9akACDkxKalRETGhr659VHIuhIy8tK0t1ZKyJSsr+NH2qgMLODAoqFAWNqaor3xVcy+Hj7+3rCzUk7MiLCz"
    "/1verAgo7MyurUxc3p6moe1J0KIaOvravA==";

// A compound file of the three streams, as the documents are.
QByteArray compoundOf(const QByteArray &wd, const QByteArray &table, const QByteArray &data)
{
    jp::cfb::File f;
    jp::cfb::Entry root;
    root.type = jp::cfb::Entry::Root;
    root.name = QStringLiteral("Root Entry");
    f.entries.push_back(root);
    for (const auto &s : {qMakePair(QStringLiteral("WordDocument"), wd), qMakePair(QStringLiteral("1Table"), table), qMakePair(QStringLiteral("Data"), data)}) {
        jp::cfb::Entry e;
        e.type = jp::cfb::Entry::Stream;
        e.name = s.first;
        e.data = s.second;
        f.entries[0].children.push_back(int(f.entries.size()));
        f.entries.push_back(e);
    }
    return jp::cfb::write(f);
}

QByteArray pattern(int n)
{
    QByteArray b(n, 0);
    for (int i = 0; i < n; ++i) b[i] = char((i * 7 + 3) & 255);
    return b;
}

QString paragraphText(const Block &b)
{
    QString t;
    for (const Inline &in : b.para.content)
        if (in.kind == Inline::Text) t += in.text;
        else if (in.kind == Inline::RunAtom && in.tag == QLatin1String("cr")) t += in.display;   // (a carriage return is kept as itself and shown as a line break)
    return t;
}

// The text of each paragraph of the body, in order (a table's cells included).
QStringList bodyTexts(const Document &d)
{
    QStringList out;
    for (const Block &b : d.body)
        if (!b.isRaw) out << paragraphText(b);
    return out;
}

bool read(const QByteArray &file, Document *d, ReadNotes *notes = nullptr, QString *error = nullptr)
{
    return doc::readDoc(file, d, error, notes);
}

const wml::KeptPart *part(const Document &d, const QString &name)
{
    for (const wml::KeptPart &p : d.parts)
        if (p.name == name) return &p;
    return nullptr;
}

// The XML of the body of the package a file comes to.
QByteArray bodyXml(const DocWriter &w)
{
    QString why;
    const QByteArray zip = doc::toDocxPackage(w.build(), &why);
    Package pkg;
    if (zip.isEmpty() || !pkg.read(zip)) return QByteArray("no package: ") + why.toUtf8();
    return pkg.part(QStringLiteral("/word/document.xml"));
}

// The sprms of a tracked insertion or deletion by the author at `by` of the file's list, on the date.
QByteArray insertion(int by, quint32 date = 0) { return sprm1(sprm::inserted, 0x81) + sprm2(sprm::insertedBy, by) + (date ? sprm4(sprm::insertedAt, date) : QByteArray()); }
QByteArray deletion(int by, quint32 date = 0) { return sprm1(sprm::deleted, 0x81) + sprm2(sprm::deletedBy, by) + (date ? sprm4(sprm::deletedAt, date) : QByteArray()); }

// Where in a stream a change reaches something the reader reads: the bytes that are not zero (most of a stream's padding and unused places are zeros,
// where a change would do nothing).
QVector<int> liveBytes(const QByteArray &s)
{
    QVector<int> out;
    for (int i = 0; i < s.size(); ++i)
        if (s[i] != 0) out.push_back(i);
    return out;
}

// The bytes of a stream's header that say how long things are and where: the lengths of the stories, and the place and length of every table the header names.
// A change to one of these (to 0xFF in its high byte, say) is a count or a length the reader must not believe.
QVector<int> fibNumbers(const QByteArray &wd)
{
    QVector<int> out;
    for (int at = 64 + 12; at <= 64 + 40; at += 4)   // (the stories' lengths)
        for (int k = 0; k < 4; ++k) out.push_back(at + k);
    for (int slot = 0; slot < 93; ++slot) {
        const int at = 154 + 8 * slot;
        if (at + 8 > wd.size()) break;
        const quint32 lcb = quint32(quint8(wd[at + 4])) | (quint32(quint8(wd[at + 5])) << 8) | (quint32(quint8(wd[at + 6])) << 16) | (quint32(quint8(wd[at + 7])) << 24);
        if (lcb == 0) continue;
        for (int k = 0; k < 8; ++k) out.push_back(at + k);   // (where the table is, and how long)
    }
    return out;
}

// The first and last bytes of every table of the table stream that the header names: a table's count of records, or the first and last of its places.
QVector<int> tableEdges(const QByteArray &wd, const QByteArray &table)
{
    QVector<int> out;
    for (int slot = 0; slot < 93; ++slot) {
        const int at = 154 + 8 * slot;
        if (at + 8 > wd.size()) break;
        auto u32 = [&](int o) { return quint32(quint8(wd[o])) | (quint32(quint8(wd[o + 1])) << 8) | (quint32(quint8(wd[o + 2])) << 16) | (quint32(quint8(wd[o + 3])) << 24); };
        const quint32 fc = u32(at), lcb = u32(at + 4);
        if (lcb == 0 || fc >= quint32(table.size())) continue;
        for (quint32 k = 0; k < 12 && k < lcb && fc + k < quint32(table.size()); ++k) out.push_back(int(fc + k));
        for (quint32 k = 1; k <= 8 && k <= lcb && fc + lcb - k < quint32(table.size()); ++k) out.push_back(int(fc + lcb - k));
    }
    return out;
}

DocWriter simple(const QString &text)
{
    DocWriter w;
    w.text = text;
    return w;
}

// ---- Open and Repair of a damaged file

// A document of 400 paragraphs (its streams are long enough to fill many sectors), and its paragraphs.
QByteArray repairableDoc(QStringList *paragraphs)
{
    QString text;
    paragraphs->clear();
    for (int i = 0; i < 400; ++i) {
        const QString p = QStringLiteral("Paragraph %1 of the repair test, with words enough to fill a line.").arg(i);
        paragraphs->push_back(p);
        text += p + QLatin1Char('\r');
    }
    return simple(text).build();
}

quint32 le32At(const QByteArray &b, int at) { return at + 4 <= b.size() ? qFromLittleEndian<quint32>(b.constData() + at) : 0; }
void setLe32(QByteArray &b, int at, quint32 v) { qToLittleEndian(v, b.data() + at); }
// The directory is the first sector of the files this writes (sector n is at (n + 1) * 512); an entry's start sector is at 116 of its 128 bytes.
int directoryEntryOf(const QByteArray &file, const QString &name)
{
    const int dir = int(le32At(file, 48));
    for (int o = (dir + 1) * 512; o + 128 <= file.size() && o < (dir + 1) * 512 + 512; o += 128) {
        const int len = qFromLittleEndian<quint16>(file.constData() + o + 64) / 2 - 1;
        if (len > 0 && QString::fromUtf16(reinterpret_cast<const char16_t *>(file.constData() + o), len) == name) return o;
    }
    return -1;
}
void zeroSector(QByteArray &f, int n) { f.replace((n + 1) * 512, 512, QByteArray(512, '\0')); }

QStringList repairedParagraphs(const QByteArray &file, QStringList *notes, QString *error = nullptr)
{
    const QByteArray fixed = doc::salvageDoc(file, notes, error);
    Document d;
    if (fixed.isEmpty() || !read(fixed, &d)) return {};
    return bodyTexts(d);
}

} // namespace

class OfficeDocTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    // ---- the text

    void textOfOnePiece()
    {
        Document d;
        QVERIFY(read(simple(QStringLiteral("Hello world\rSecond paragraph\r")).build(), &d));
        QCOMPARE(bodyTexts(d), (QStringList{"Hello world", "Second paragraph"}));
    }

    void eightBitTextIsTheWesternCodePage()
    {
        // The curly quotes, the dashes, the euro sign, and the rest are bytes 0x80 to 0x9F of the stored text, not their Latin-1 meanings.
        const QString text = QStringLiteral("“quoted” – €5 … ™ šž é ‘x’\r");
        DocWriter w = simple(text);
        const QByteArray file = w.build();
        QVERIFY(file.contains('\x93') && file.contains('\x94') && file.contains('\x96') && file.contains('\x80') && file.contains('\x85'));
        Document d;
        QVERIFY(read(file, &d));
        QCOMPARE(bodyTexts(d), QStringList{text.chopped(1)});
        // (And the same text in two pieces, the first 8-bit and the second 16-bit.)
        DocWriter v = simple(text);
        v.splitAt = 12;
        Document e;
        QVERIFY(read(v.build(), &e));
        QCOMPARE(bodyTexts(e), QStringList{text.chopped(1)});
    }

    void textInTwoPiecesOfDifferentCodings()
    {
        DocWriter w = simple(QStringLiteral("café first\r€ and Ω second\r"));
        w.splitAt = 11;   // after "café first\r"
        Document d;
        QVERIFY(read(w.build(), &d));
        QCOMPARE(bodyTexts(d), (QStringList{"café first", "€ and Ω second"}));
        // (The same text kept as 16-bit throughout reads the same.)
        DocWriter u = simple(w.text);
        u.unicode = true;
        Document e;
        QVERIFY(read(u.build(), &e));
        QCOMPARE(bodyTexts(e), bodyTexts(d));
    }

    void controlCharactersInText()
    {
        Document d;
        QVERIFY(read(simple(QStringLiteral("a\tb\x0Bz\x0C""c\x1E""d\r")).build(), &d));
        // (A tab, a line break, and a page break stay in the text; a no-break hyphen is its own character.)
        QCOMPARE(bodyTexts(d), (QStringList{QStringLiteral("a\tb z\fc‑d")}));
    }

    void aParagraphMarkInsideAParagraphIsALineBreak()
    {
        // One stretch of paragraph properties covers both marks: the first is a line break, as the original program reads it.
        DocWriter w = simple(QStringLiteral("one\rtwo\rthree\r"));
        w.paras = {{0, 8, 0, QByteArray()}, {8, 14, 0, QByteArray()}};
        Document d;
        QVERIFY(read(w.build(), &d));
        QCOMPARE(bodyTexts(d), (QStringList{QStringLiteral("one\u2028two"), QStringLiteral("three")}));
    }

    void aParagraphWithMarksInsideHasThePropertiesItBeginsWith()
    {
        // The stretches of properties don't line up with the marks: one covers "one", the next the mark and a space, the last "two" and its mark.
        DocWriter w = simple(QStringLiteral("one\r two\r"));
        w.paras = {{0, 3, 0, sprm1(sprm::align, 1)}, {3, 5, 0, QByteArray()}, {5, 9, 0, sprm1(sprm::align, 2)}};
        Document d;
        QVERIFY(read(w.build(), &d));
        QCOMPARE(bodyTexts(d), QStringList{QStringLiteral("one\u2028 two")});
        QCOMPARE(d.body[0].para.props.jc, QStringLiteral("center"));
    }

    void everyPartOfThePackageIsWellFormedXml()
    {
        // Names and texts out of a file may hold anything: control characters, quotes, and angle brackets.
        DocWriter w = simple(QStringLiteral("a<b & \"c\" \x01\x02 d\r"));
        w.fonts = {QStringLiteral("Bad \"Font\" <&>\x03"), QStringLiteral("Times New Roman")};
        StyleSpec normal;
        normal.name = QStringLiteral("Normal");
        normal.sti = 0;
        StyleSpec odd;
        odd.name = QStringLiteral("odd <style> \"x\" & \x04\x05");
        w.styles = {normal, odd};
        ListLevelSpec lvl;
        lvl.text = QString(QChar(0)) + QStringLiteral("\x0B\x0C<\"&");
        w.lists = {{lvl}};
        w.paras = {{0, 20, 1, sprm2(sprm::listInstance, 1)}};
        w.chars = {{0, 2, sprm2(sprm::fontAscii, 0)}};
        QString why;
        const QByteArray zip = doc::toDocxPackage(w.build(), &why);
        QVERIFY2(!zip.isEmpty(), qPrintable(why));
        Package pkg;
        QVERIFY(pkg.read(zip));
        int parts = 0;
        for (const QString &name : pkg.partNames()) {
            if (!(name.endsWith(QLatin1String(".xml")) || name.endsWith(QLatin1String(".rels")))) continue;
            QXmlStreamReader r(pkg.part(name));
            while (!r.atEnd()) r.readNext();
            QVERIFY2(!r.hasError(), qPrintable(name + QLatin1String(": ") + r.errorString()));
            ++parts;
        }
        QVERIFY(parts >= 5);
        Document d;
        QVERIFY(read(w.build(), &d));
    }

    void compatibilityModeIsThatOfTheOlderPrograms()
    {
        Document d;
        QVERIFY(read(simple(QStringLiteral("x\r")).build(), &d));
        QCOMPARE(d.compatMode, 11);
    }

    // The older compatibility bits are in the byte at 84 of the document's settings ([MS-DOC] DopBase): bit 3 is fWrapTrailSpaces, the spaces that end a paragraph wrapping to the next
    // line (w:wrapTrailSpaces), and bit 0 the hanging indent that is no tab stop (w:noTabHangInd); a file that sets neither has neither.
    void trailingSpacesThatWrapAreSaid()
    {
        auto compat = [&](int byte84) {
            DocWriter w = simple(QStringLiteral("x\r"));
            w.compatByte84 = byte84;
            QString why;
            const QByteArray zip = doc::toDocxPackage(w.build(), &why);
            Package pkg;
            if (!pkg.read(zip)) return QByteArray();
            return pkg.part(QStringLiteral("/word/settings.xml"));
        };
        const QByteArray both = compat(0x08 | 0x01), one = compat(0x01), none = compat(0);
        QVERIFY(!both.isEmpty());
        QVERIFY2(wml::readCompatOptions(both).wrapTrailSpaces, both.constData());
        QVERIFY(both.contains("noTabHangInd"));
        QVERIFY(!wml::readCompatOptions(one).wrapTrailSpaces && one.contains("noTabHangInd"));
        QVERIFY(!wml::readCompatOptions(none).wrapTrailSpaces);
    }

    // ---- properties

    void characterProperties()
    {
        DocWriter w = simple(QStringLiteral("plain bold both\r"));
        w.fonts = {QStringLiteral("Times New Roman"), QStringLiteral("Arial")};
        w.chars = {{6, 10, sprm1(sprm::bold, 1)}, {11, 15, sprm1(sprm::bold, 1) + sprm1(sprm::italic, 1) + sprm2(sprm::size, 28) + sprm2(sprm::fontAscii, 1) + sprm2(sprm::fontOther, 1)}};
        Document d;
        QVERIFY(read(w.build(), &d));
        const Block &p = d.body.first();
        QCOMPARE(paragraphText(p), QStringLiteral("plain bold both"));
        QVector<Inline> runs;
        for (const Inline &in : p.para.content)
            if (in.kind == Inline::Text) runs << in;
        QCOMPARE(runs.size(), 4);
        QVERIFY(!runs[0].props.bold.value_or(false));
        QCOMPARE(runs[1].text, QStringLiteral("bold"));
        QVERIFY(runs[1].props.bold.value_or(false));
        QVERIFY(!runs[2].props.bold.value_or(false));
        QCOMPARE(runs[3].text, QStringLiteral("both"));
        QVERIFY(runs[3].props.bold.value_or(false) && runs[3].props.italic.value_or(false));
        QCOMPARE(runs[3].props.size.value_or(0), 28);
        QVERIFY(runs[3].props.fonts && runs[3].props.fonts->ascii == QLatin1String("Arial"));
    }

    void trackedChangesHaveTheirAuthorsAndDates()
    {
        DocWriter w = simple(QStringLiteral("a new b old c\r"));
        w.revisionAuthors = {QStringLiteral("Ann"), QStringLiteral("Bob")};
        w.chars = {{2, 5, insertion(0, dttm(2003, 4, 7, 9, 30))}, {8, 11, deletion(1, dttm(2004, 11, 23, 16, 5))}};
        const QByteArray x = bodyXml(w);
        QVERIFY2(x.contains("<w:ins w:id=\"1\" w:author=\"Ann\" w:date=\"2003-04-07T09:30:00Z\"><w:r><w:t xml:space=\"preserve\">new</w:t></w:r></w:ins>"), x.constData());
        QVERIFY2(x.contains("<w:del w:id=\"2\" w:author=\"Bob\" w:date=\"2004-11-23T16:05:00Z\"><w:r><w:delText xml:space=\"preserve\">old</w:delText></w:r></w:del>"), x.constData());
        // A date the file leaves out, or one that is no date (month 0), is left out of the mark.
        DocWriter v = simple(QStringLiteral("a new b old c\r"));
        v.revisionAuthors = {QStringLiteral("Ann")};
        v.chars = {{2, 5, insertion(0)}, {8, 11, deletion(0, dttm(2003, 0, 7, 9, 30))}};
        const QByteArray y = bodyXml(v);
        QVERIFY2(y.contains("<w:ins w:id=\"1\" w:author=\"Ann\"><w:r>"), y.constData());
        QVERIFY2(y.contains("<w:del w:id=\"2\" w:author=\"Ann\"><w:r>"), y.constData());
    }

    void aParagraphMarkThatWasInsertedOrDeletedSaysSo()
    {
        DocWriter w = simple(QStringLiteral("first\rsecond\r"));
        w.revisionAuthors = {QStringLiteral("Ann")};
        w.chars = {{5, 6, insertion(0, dttm(2003, 4, 7, 9, 30))}, {12, 13, deletion(0, dttm(2004, 11, 23, 16, 5))}};
        const QByteArray x = bodyXml(w);
        QVERIFY2(x.contains("<w:pPr><w:rPr><w:ins w:id=\"1\" w:author=\"Ann\" w:date=\"2003-04-07T09:30:00Z\"/></w:rPr></w:pPr>"), x.constData());
        QVERIFY2(x.contains("<w:pPr><w:rPr><w:del w:id=\"2\" w:author=\"Ann\" w:date=\"2004-11-23T16:05:00Z\"/></w:rPr></w:pPr>"), x.constData());
        // (The words of the paragraphs are not marked: only their marks are.)
        QVERIFY(!x.contains("<w:ins w:id=\"1\" w:author=\"Ann\" w:date=\"2003-04-07T09:30:00Z\"><w:r>"));
        // And the document reads them: the mark's properties are kept with the paragraph.
        Document d;
        QVERIFY(read(w.build(), &d));
        QCOMPARE(bodyTexts(d), (QStringList{QStringLiteral("first"), QStringLiteral("second")}));
        QVERIFY(d.body[0].para.props.markRun.has_value());
    }

    void plainCharactersKeepTheirTrackedChange()
    {
        // "Plain" takes the formatting back to the style's; it is no reason to forget who typed the words.
        DocWriter w = simple(QStringLiteral("a new b\r"));
        w.revisionAuthors = {QStringLiteral("Ann")};
        w.chars = {{2, 5, insertion(0, dttm(2003, 4, 7, 9, 30)) + sprm1(0x2A33, 1) + sprm1(sprm::bold, 1)}};
        const QByteArray x = bodyXml(w);
        QVERIFY2(x.contains("<w:ins w:id=\"1\" w:author=\"Ann\" w:date=\"2003-04-07T09:30:00Z\"><w:r><w:rPr><w:b/></w:rPr><w:t xml:space=\"preserve\">new</w:t></w:r></w:ins>"), x.constData());
    }

    void textInsertedAndThenDeletedKeepsBothMarks()
    {
        DocWriter w = simple(QStringLiteral("a new b\r"));
        w.revisionAuthors = {QStringLiteral("Ann"), QStringLiteral("Bob")};
        w.chars = {{2, 5, insertion(0, dttm(2003, 4, 7, 9, 30)) + deletion(1, dttm(2004, 11, 23, 16, 5))}};
        const QByteArray x = bodyXml(w);
        QVERIFY2(x.contains("<w:ins w:id=\"1\" w:author=\"Ann\" w:date=\"2003-04-07T09:30:00Z\"><w:del w:id=\"2\" w:author=\"Bob\" w:date=\"2004-11-23T16:05:00Z\">"
                            "<w:r><w:delText xml:space=\"preserve\">new</w:delText></w:r></w:del></w:ins>"),
                 x.constData());
    }

    void toggleRelativeToTheStyle()
    {
        // A style that is bold; a run stated "the opposite of the style" is not bold.
        DocWriter w = simple(QStringLiteral("abc\r"));
        StyleSpec normal;
        normal.name = QStringLiteral("Normal");
        normal.sti = 0;
        StyleSpec head;
        head.name = QStringLiteral("heading 1");
        head.sti = 1;
        head.base = 0;
        head.character = sprm1(sprm::bold, 1);
        w.styles = {normal, head};
        w.paras = {{0, 4, 1, QByteArray()}};
        w.chars = {{1, 2, sprm1(sprm::bold, 0x81)}};
        Document d;
        QVERIFY(read(w.build(), &d));
        const Block &p = d.body.first();
        QCOMPARE(p.para.props.style, QStringLiteral("Heading1"));
        QVector<Inline> runs;
        for (const Inline &in : p.para.content)
            if (in.kind == Inline::Text) runs << in;
        QCOMPARE(runs.size(), 3);
        QVERIFY(!runs[0].props.bold.has_value());          // (as the style has it)
        QCOMPARE(runs[1].props.bold.value_or(true), false);   // (the opposite)
    }

    void paragraphProperties()
    {
        DocWriter w = simple(QStringLiteral("centered\rindented\r"));
        w.paras = {{0, 9, 0, sprm1(sprm::align, 1) + sprm2(sprm::before, 120) + sprm2(sprm::after, 240)},
                   {9, 18, 0, sprm2(sprm::left, 720) + sprm2(sprm::firstLine, 0xFE98 /* -360 */) + sprm1(sprm::keepNext, 1)}};
        Document d;
        QVERIFY(read(w.build(), &d));
        const wml::ParaProps &a = d.body[0].para.props, &b = d.body[1].para.props;
        QCOMPARE(a.jc, QStringLiteral("center"));
        QCOMPARE(a.before.value_or(-1), 120);
        QCOMPARE(a.after.value_or(-1), 240);
        QCOMPARE(b.indLeft.value_or(-1), 720);
        QCOMPARE(b.indFirst.value_or(0), -360);
        QVERIFY(b.keepNext.value_or(false));
    }

    void stylesInheritFromTheirBase()
    {
        DocWriter w = simple(QStringLiteral("Title text\r"));
        StyleSpec normal;
        normal.name = QStringLiteral("Normal");
        normal.sti = 0;
        normal.character = sprm2(sprm::size, 24);
        StyleSpec head;
        head.name = QStringLiteral("heading 1");
        head.sti = 1;
        head.base = 0;
        head.paragraph = sprm1(sprm::align, 1);
        head.character = sprm1(sprm::bold, 1);
        StyleSpec mine;
        mine.name = QStringLiteral("My Style");
        mine.base = 1;
        mine.paragraph = sprm2(sprm::left, 360);
        w.styles = {normal, head, mine};
        w.paras = {{0, 11, 2, QByteArray()}};
        Document d;
        QVERIFY(read(w.build(), &d));
        const wml::Style *s = d.styles.find(QStringLiteral("MyStyle"));
        QVERIFY(s);
        QCOMPARE(s->basedOn, QStringLiteral("Heading1"));
        const wml::ParaProps resolved = d.styles.paragraph(QStringLiteral("MyStyle"));
        QCOMPARE(resolved.jc, QStringLiteral("center"));   // from the style it is based on
        QCOMPARE(resolved.indLeft.value_or(-1), 360);      // its own
        QCOMPARE(d.styles.run(QStringLiteral("MyStyle"), QString()).bold.value_or(false), true);
        QCOMPARE(d.styles.run(QStringLiteral("MyStyle"), QString()).size.value_or(0), 24);   // from Normal, two styles up
        QCOMPARE(d.body.first().para.props.style, QStringLiteral("MyStyle"));
    }

    void aStyleThatIsItsOwnAncestorIsRead()
    {
        DocWriter w = simple(QStringLiteral("x\r"));
        StyleSpec a;
        a.name = QStringLiteral("Normal");
        a.sti = 0;
        a.base = 1;
        StyleSpec b;
        b.name = QStringLiteral("Other");
        b.base = 0;
        b.character = sprm1(sprm::bold, 0x81);
        w.styles = {a, b};
        w.paras = {{0, 2, 1, QByteArray()}};
        Document d;
        QVERIFY(read(w.build(), &d));
        // (Counted: the steps along style chains, not the time.)
        QVERIFY2(doc::readWorkForTest().styleSteps <= 3 * doc::kMaxStyleDepth, qPrintable(QString::number(doc::readWorkForTest().styleSteps)));
        QCOMPARE(bodyTexts(d), QStringList{"x"});
    }

    // ---- lists

    void numberedAndBulletedLists()
    {
        DocWriter w = simple(QStringLiteral("one\rtwo\rbullet\r"));
        ListLevelSpec dec;
        dec.format = 0;
        dec.text = QString(QChar(0)) + QLatin1Char('.');
        dec.paragraph = sprm2(sprm::left, 720) + sprm2(sprm::firstLine, 0xFE98);
        ListLevelSpec bul;
        bul.format = 23;
        bul.text = QString(QChar(0xF0B7));
        w.lists = {{dec}, {bul}};
        w.paras = {{0, 4, 0, sprm2(sprm::listInstance, 1) + sprm1(sprm::listLevel, 0)}, {4, 8, 0, sprm2(sprm::listInstance, 1)}, {8, 15, 0, sprm2(sprm::listInstance, 2)}};
        Document d;
        QVERIFY(read(w.build(), &d));
        QCOMPARE(d.body[0].para.props.numId.value_or(-1), 1);
        QCOMPARE(d.body[1].para.props.numId.value_or(-1), 1);
        QCOMPARE(d.body[2].para.props.numId.value_or(-1), 2);
        const wml::AbstractNum *first = d.numbering.abstractOf(1), *second = d.numbering.abstractOf(2);
        QVERIFY(first && second);
        QCOMPARE(first->levels[0].numFmt, QStringLiteral("decimal"));
        QCOMPARE(first->levels[0].text, QStringLiteral("%1."));
        QCOMPARE(first->levels[0].pPr.indLeft.value_or(-1), 720);
        QCOMPARE(first->levels[0].pPr.indFirst.value_or(0), -360);
        QCOMPARE(second->levels[0].numFmt, QStringLiteral("bullet"));
        QCOMPARE(second->levels[0].text, QString(QChar(0xF0B7)));
    }

    void aParagraphInAListThatIsNotThereIsNotInOne()
    {
        DocWriter w = simple(QStringLiteral("one\r"));
        w.paras = {{0, 4, 0, sprm2(sprm::listInstance, 9)}};
        Document d;
        QVERIFY(read(w.build(), &d));
        QVERIFY(!d.body[0].para.props.numId.has_value() || *d.body[0].para.props.numId == 0);
    }

    // ---- sections, headers, and footers

    void sectionsAndPageSetup()
    {
        // Two sections: the first ends in a form feed of its own paragraph; the second is landscape.
        DocWriter w = simple(QStringLiteral("first\r\fsecond\r"));
        const QByteArray one = sprm2(sprm::pageWidth, 12240) + sprm2(sprm::pageHeight, 15840) + sprm2(sprm::marginLeft, 1800) + sprm2(sprm::marginRight, 1800) + sprm2(sprm::marginTop, 1440) + sprm2(sprm::marginBottom, 1440);
        const QByteArray two = sprm1(sprm::landscape, 2) + sprm2(sprm::pageWidth, 15840) + sprm2(sprm::pageHeight, 12240) + sprm2(sprm::columns, 1) + sprm2(sprm::columnSpace, 360) + sprm1(sprm::sectionType, 0);
        w.sections = {{7, one}, {int(w.text.size()) + 1, two}};
        w.paras = {{0, 6, 0, QByteArray()}, {6, 7, 0, QByteArray()}, {7, int(w.text.size()), 0, QByteArray()}};
        Document d;
        QVERIFY(read(w.build(), &d));
        QVERIFY(d.body[1].para.props.section);   // the form feed's paragraph ends the first section
        QCOMPARE(d.body[1].para.props.section->pageW, 12240);
        QCOMPARE(d.body[1].para.props.section->left, 1800);
        QVERIFY(d.lastSectionStated);
        QCOMPARE(d.lastSection.pageW, 15840);
        QCOMPARE(d.lastSection.pageH, 12240);
        QCOMPARE(d.lastSection.orient, QStringLiteral("landscape"));
        QCOMPARE(d.lastSection.cols, 2);
        QCOMPARE(d.lastSection.colSpace, 360);
        QCOMPARE(d.lastSection.type, QStringLiteral("continuous"));
    }

    // A section's line numbering starts at the number the file's start sprm says, plus one (a start of 4 numbers the first line 5), and the .docx written from it says what
    // that format's start means, the numbers skipped: 4.
    void aSectionsLineNumberStartIsTheNumbersSkipped()
    {
        DocWriter w = simple(QStringLiteral("first\r"));
        const QByteArray numbers = sprm2(0x5015, 1) + sprm1(0x3013, 2) + sprm2(0x501B, 4);   // countBy 1, continuous, start 4
        w.sections = {{int(w.text.size()) + 1, numbers}};
        w.paras = {{0, int(w.text.size()), 0, QByteArray()}};
        Document d;
        QVERIFY(read(w.build(), &d));
        QVERIFY(d.lastSectionStated);
        QByteArray numbering;
        for (const wml::RawChild &child : d.lastSection.extra)
            if (child.xml.contains("lnNumType")) numbering = child.xml;
        QVERIFY2(numbering.contains("w:start=\"4\""), numbering.constData());
        QVERIFY2(numbering.contains("w:restart=\"continuous\""), numbering.constData());
    }

    void aFormFeedInAParagraphIsAPageBreak()
    {
        Document d;
        QVERIFY(read(simple(QStringLiteral("before\fafter\r")).build(), &d));
        QCOMPARE(d.body.size(), qsizetype(1));
        QCOMPARE(bodyTexts(d), QStringList{QStringLiteral("before\fafter")});
        QVERIFY(!d.body[0].para.props.section);
    }

    void headersAndFootersOfASection()
    {
        // Six stories for the notes' separators, then six for the section: even header, odd header, even footer, odd footer, first header, first footer.
        DocWriter w = simple(QStringLiteral("body\r"));
        w.headerStories = {QString(), QString(), QString(), QString(), QString(), QString(),
                           QString(), QStringLiteral("Header text\r"), QString(), QStringLiteral("Footer text\r"), QStringLiteral("First page\r"), QString()};
        w.sections = {{5, sprm1(sprm::titlePage, 1)}};
        Document d;
        QVERIFY(read(w.build(), &d));
        QVERIFY(d.lastSectionStated);
        QVERIFY(d.lastSection.titlePg);
        const wml::HeaderRefs refs = wml::headerRefsOf(d.lastSection);
        QVERIFY(refs.any());
        QVERIFY(refs.header[0].isEmpty() == false);
        QVERIFY(refs.footer[0].isEmpty() == false);
        QVERIFY(refs.header[1].isEmpty() == false);
        QVERIFY(refs.footer[1].isEmpty());       // (no first-page footer)
        QVERIFY(refs.header[2].isEmpty());       // (no even header)
        auto text = [&](const QString &rid) {
            for (const auto &r : d.mainRelationships)
                if (r.id == rid)
                    if (const wml::KeptPart *p = part(d, r.target)) {
                        QVector<Block> blocks;
                        wml::readBlocks(p->data, &blocks);
                        QStringList t;
                        for (const Block &b : blocks) t << paragraphText(b);
                        return t.join(QLatin1Char('|'));
                    }
            return QString();
        };
        // (Each story ends in one mark more than it has paragraphs: that one is not a paragraph.)
        QCOMPARE(text(refs.header[0]), QStringLiteral("Header text"));
        QCOMPARE(text(refs.footer[0]), QStringLiteral("Footer text"));
        QCOMPARE(text(refs.header[1]), QStringLiteral("First page"));
    }

    void evenAndOddHeadersAreSaid()
    {
        DocWriter w = simple(QStringLiteral("x\r"));
        w.evenAndOddHeaders = true;
        Document d;
        QVERIFY(read(w.build(), &d));
        QVERIFY(d.evenAndOddHeaders);
        QCOMPARE(d.defaultTabStop, 720);
        DocWriter v = simple(QStringLiteral("x\r"));
        v.defaultTab = 360;
        Document e;
        QVERIFY(read(v.build(), &e));
        QVERIFY(!e.evenAndOddHeaders);
        QCOMPARE(e.defaultTabStop, 360);
    }

    // ---- notes

    void footnotes()
    {
        DocWriter w = simple(QStringLiteral("Text with a note.\r"));
        w.chars = {{16, 17, sprm1(sprm::special, 1)}};
        w.text = QStringLiteral("Text with a note\x02.\r");
        w.chars = {{16, 17, sprm1(sprm::special, 1)}};
        w.footnotes = {QStringLiteral("\x02 The note itself.\r")};
        w.footnoteRefs = {16};
        Document d;
        QVERIFY(read(w.build(), &d));
        QCOMPARE(bodyTexts(d), QStringList{QStringLiteral("Text with a note.")});
        bool cited = false;
        for (const Inline &in : d.body[0].para.content)
            if (in.kind == Inline::RunAtom && in.tag == QLatin1String("footnoteReference")) cited = true;
        QVERIFY(cited);
        const wml::KeptPart *notes = part(d, QStringLiteral("/word/footnotes.xml"));
        QVERIFY(notes);
        QVector<wml::NotePart> list;
        QVERIFY(wml::readNotes(notes->data, &list));
        QString found;
        for (const wml::NotePart &n : list)
            if (n.type.isEmpty())
                for (const Block &b : n.blocks) found += paragraphText(b);
        QCOMPARE(found.trimmed(), QStringLiteral("The note itself."));
    }

    // ---- bookmarks

    void bookmarksAreWhereTheFileSays()
    {
        DocWriter w = simple(QStringLiteral("Hello world\rSecond\r"));
        w.bookmarks = {{QStringLiteral("Greeting"), 0, 5}, {QStringLiteral("Second_one"), 12, 18}};
        Document d;
        QVERIFY(read(w.build(), &d));
        QCOMPARE(bodyTexts(d), (QStringList{"Hello world", "Second"}));
        auto marks = [&](int paragraph) {
            QStringList out;
            QString text;
            for (const Inline &in : d.body[paragraph].para.content) {
                if (in.kind == Inline::Text) text += in.text;
                else if (in.kind == Inline::ParaAtom && (in.tag == QLatin1String("bookmarkStart") || in.tag == QLatin1String("bookmarkEnd")))
                    out << QStringLiteral("%1@%2").arg(in.tag.mid(8), QString::number(text.size()));
            }
            return out;
        };
        QCOMPARE(marks(0), (QStringList{"Start@0", "End@5"}));
        QCOMPARE(marks(1), (QStringList{"Start@0", "End@6"}));
        QVERIFY(QString::fromUtf8(d.body[0].para.content.first().raw).contains(QLatin1String("Greeting")));
    }

    // ---- fields

    void aFieldKeepsItsInstructionAndShowsItsResult()
    {
        const QString text = QStringLiteral("Page \x13 PAGE \x14""7\x15 of \x13 NUMPAGES \x14""9\x15\r");
        Document d;
        QVERIFY(read(simple(text).build(), &d));
        // The result is text; the field's marks and instruction are kept (so PAGE and NUMPAGES stay live).
        QCOMPARE(bodyTexts(d), QStringList{QStringLiteral("Page 7 of 9")});
        QStringList instructions;
        int marks = 0;
        for (const Inline &in : d.body[0].para.content) {
            if (in.kind != Inline::RunAtom) continue;
            if (in.tag == QLatin1String("fldChar")) ++marks;
            if (in.tag == QLatin1String("instrText")) instructions << QString::fromUtf8(in.raw);
        }
        QCOMPARE(marks, 6);
        QCOMPARE(instructions.size(), 2);
        QVERIFY(instructions[0].contains(QLatin1String("PAGE")));
        QVERIFY(instructions[1].contains(QLatin1String("NUMPAGES")));
    }

    void fieldsThatNestTooDeepAreStillBalanced()
    {
        QString text = QStringLiteral("a");
        const int depth = doc::kMaxFieldDepth + 30;
        for (int i = 0; i < depth; ++i) text += QStringLiteral(" \x13 X \x14");
        text += QStringLiteral("deep");
        for (int i = 0; i < depth; ++i) text += QLatin1Char(0x15);
        text += QStringLiteral("\r");
        Document d;
        QVERIFY(read(simple(text).build(), &d));
        int begins = 0, ends = 0;
        for (const Inline &in : d.body[0].para.content)
            if (in.kind == Inline::RunAtom && in.tag == QLatin1String("fldChar")) {
                if (in.key == QLatin1String("begin")) ++begins;
                if (in.key == QLatin1String("end")) ++ends;
            }
        QCOMPARE(begins, ends);
        QVERIFY(begins <= doc::kMaxFieldDepth);
        QVERIFY(paragraphText(d.body[0]).contains(QLatin1String("deep")));
    }

    void fieldsOpenedByTheHundredThousandAreBalancedAndCostLittle()
    {
        // One mark opens a field, and the fields of a file can be nested in as many as it has marks (a state kept for each was a state kept for 40 million).
        const int depth = 200000;
        QString text = QStringLiteral("start");
        text += QString(depth, QChar(0x13));
        text += QString(depth, QChar(0x15));
        text += QStringLiteral(" finish\r");
        Document d;
        QVERIFY(read(simple(text).build(), &d));
        // (Each mark is converted once: the work is the length of the text, not a state kept for each mark.)
        QVERIFY2(doc::readWorkForTest().charsConverted <= text.size() && doc::readWorkForTest().outputElements < 1000, qPrintable(QString::number(doc::readWorkForTest().charsConverted)));
        int begins = 0, ends = 0;
        for (const Inline &in : d.body[0].para.content)
            if (in.kind == Inline::RunAtom && in.tag == QLatin1String("fldChar")) {
                if (in.key == QLatin1String("begin")) ++begins;
                if (in.key == QLatin1String("end")) ++ends;
            }
        QCOMPARE(begins, ends);
        QVERIFY(begins <= doc::kMaxFieldDepth);
        // (Every field is closed again, so what follows them is text in the paragraph.)
        QCOMPARE(paragraphText(d.body[0]), QStringLiteral("start finish"));
    }

    void aFieldLeftOpenIsClosed()
    {
        Document d;
        QVERIFY(read(simple(QStringLiteral("open \x13 PAGE \x14""3\r")).build(), &d));
        int begins = 0, ends = 0;
        for (const Inline &in : d.body[0].para.content)
            if (in.kind == Inline::RunAtom && in.tag == QLatin1String("fldChar")) {
                if (in.key == QLatin1String("begin")) ++begins;
                if (in.key == QLatin1String("end")) ++ends;
            }
        QCOMPARE(begins, 1);
        QCOMPARE(ends, 1);
    }

    // ---- tables

    void aTableWithRowsAndCells()
    {
        // Two rows of two cells: a cell's last paragraph ends in \a, a row ends in a \a paragraph that says where the cells are.
        const QString text = QStringLiteral("a1\ab1\a\ra2\ab2\a\rafter\r");
        DocWriter w = simple(text);
        const QByteArray in = sprm1(sprm::inTable, 1) + sprm4(sprm::tableDepth, 1);
        const QByteArray row = in + sprm1(sprm::endsRow, 1) + tableRow({0, 2000, 5000});
        w.paras = {{0, 3, 0, in}, {3, 6, 0, in}, {6, 7, 0, row}, {7, 10, 0, in}, {10, 13, 0, in}, {13, 14, 0, row}, {14, 20, 0, QByteArray()}};
        Document d;
        QVERIFY(read(w.build(), &d));
        QStringList cells;
        int rows = 0, tables = 0;
        QVector<int> grid;
        for (const Block &b : d.body) {
            if (b.isRaw && b.tag == QLatin1String("tbl") && b.role == Inline::Open) {
                ++tables;
                if (b.table) grid = b.table->grid;
            }
            if (b.isRaw && b.tag == QLatin1String("tr") && b.role == Inline::Open) ++rows;
            if (!b.isRaw) cells << paragraphText(b);
        }
        QCOMPARE(tables, 1);
        QCOMPARE(rows, 2);
        QCOMPARE(cells, (QStringList{"a1", "b1", "a2", "b2", "after"}));
        QCOMPARE(grid, (QVector<int>{2000, 3000}));
    }

    void tablesNestedTooDeepArePlainParagraphs()
    {
        // Nested tables to twice the limit: nothing is lost, and nothing nests past the limit.
        const int depth = doc::kMaxTableDepth * 2;
        QString text;
        DocWriter w;
        int at = 0;
        for (int level = 1; level <= depth; ++level) {
            const QByteArray in = sprm1(sprm::inTable, 1) + sprm4(sprm::tableDepth, quint32(level));
            text += QStringLiteral("c%1\a").arg(level);
            w.paras.push_back({at, int(text.size()), 0, in});
            at = int(text.size());
        }
        for (int level = depth; level >= 1; --level) {
            const QByteArray row = sprm1(sprm::inTable, 1) + sprm4(sprm::tableDepth, quint32(level)) + sprm1(sprm::endsRow, 1) + tableRow({0, 3000});
            text += QStringLiteral("\a");
            w.paras.push_back({at, int(text.size()), 0, row});
            at = int(text.size());
        }
        text += QStringLiteral("end\r");
        w.text = text;
        w.paras.push_back({at, int(text.size()), 0, QByteArray()});
        Document d;
        ReadNotes notes;
        QVERIFY(read(w.build(), &d, &notes));
        int open = 0, deepest = 0;
        for (const Block &b : d.body) {
            if (b.isRaw && b.tag == QLatin1String("tbl")) {
                open += b.role == Inline::Open ? 1 : -1;
                deepest = std::max(deepest, open);
            }
        }
        QVERIFY(deepest <= doc::kMaxTableDepth);
        QVERIFY(bodyTexts(d).join(QLatin1Char(' ')).contains(QLatin1String("end")));
        QVERIFY(bodyTexts(d).join(QLatin1Char(' ')).contains(QStringLiteral("c%1").arg(depth)));
    }

    // ---- pictures

    void aPictureInTheText()
    {
        DocWriter w = simple(QStringLiteral("a\x01""b\r"));
        const int at = w.addPicture(tinyPng(8, 6), 2880, 1440);   // two inches by one
        w.chars = {{1, 2, sprm1(sprm::special, 1) + sprm4(sprm::picture, quint32(at))}};
        Document d;
        QVERIFY(read(w.build(), &d));
        QCOMPARE(bodyTexts(d), QStringList{QStringLiteral("ab")});
        const Inline *drawing = nullptr;
        for (const Inline &in : d.body[0].para.content)
            if (in.kind == Inline::RunAtom && in.tag == QLatin1String("drawing")) drawing = &in;
        QVERIFY(drawing);
        const QVector<wml::Shape> shapes = wml::readShapes(drawing->raw);
        QCOMPARE(shapes.size(), qsizetype(1));
        QCOMPARE(shapes[0].kind, wml::Shape::Picture);
        QVERIFY(!shapes[0].floating);
        QCOMPARE(shapes[0].width, 144.0);
        QCOMPARE(shapes[0].height, 72.0);
        QVERIFY(!shapes[0].embed.isEmpty());
        bool media = false;
        for (const wml::KeptPart &p : d.parts)
            if (p.name.startsWith(QLatin1String("/word/media/")) && p.data.startsWith(QByteArray("\x89PNG"))) media = true;
        QVERIFY(media);
    }

    void aPictureWithNoSizeTakesItsSizeFromItsHeader()
    {
        // A picture the file gives no size for is shown at its own. Its header says what that is; the whole image was decoded to find out, which for a small
        // file that makes a very large image (and a picture for each of thousands of places in it) is all the memory and time there are. With the allowance
        // for decoding turned down to one megabyte this 2000 by 2000 picture can't be decoded, and still has its size.
        QImage image(2000, 2000, QImage::Format_Grayscale8);
        image.fill(200);
        QByteArray png;
        {
            QBuffer buffer(&png);
            QVERIFY(buffer.open(QIODevice::WriteOnly));
            QVERIFY(image.save(&buffer, "PNG"));
        }
        struct Allowance {
            int before = QImageReader::allocationLimit();
            Allowance() { QImageReader::setAllocationLimit(1); }
            ~Allowance() { QImageReader::setAllocationLimit(before); }
        } allowance;
        DocWriter w = simple(QStringLiteral("a\x01""b\r"));
        const int at = w.addPicture(png, 0, 0);
        w.chars = {{1, 2, sprm1(sprm::special, 1) + sprm4(sprm::picture, quint32(at))}};
        const QByteArray x = bodyXml(w);
        // (2000 pixels at 96 to the inch: 1500 points, in the 12,700ths of a point the package counts in.)
        QVERIFY2(x.contains("<wp:extent cx=\"19050000\" cy=\"19050000\"/>"), x.constData());
    }

    void aRuleInTheLineIsAShapeWithNoPicture()
    {
        DocWriter w = simple(QStringLiteral("a\x01""b\r"));
        const int at = w.addRule(30);   // one and a half points high
        w.chars = {{1, 2, sprm1(sprm::special, 1) + sprm4(sprm::picture, quint32(at))}};
        Document d;
        ReadNotes notes;
        QVERIFY(read(w.build(), &d, &notes));
        QVERIFY(!notes.leftOut);
        const Inline *pict = nullptr;
        for (const Inline &in : d.body[0].para.content)
            if (in.kind == Inline::RunAtom && in.tag == QLatin1String("pict")) pict = &in;
        QVERIFY(pict);
        QVERIFY(QString::fromUtf8(pict->raw).contains(QLatin1String("o:hr=\"t\"")));
        const QVector<wml::Shape> shapes = wml::readShapes(pict->raw);
        QCOMPARE(shapes.size(), qsizetype(1));
        QCOMPARE(shapes[0].kind, wml::Shape::Rect);
        QVERIFY(!shapes[0].floating);
        QCOMPARE(shapes[0].height, 1.5);
        QVERIFY(shapes[0].filled);
        QCOMPARE(shapes[0].fillColor, QStringLiteral("808080"));
    }

    void thePageNumberRestartsOnlyWhereTheSectionSaysSo()
    {
        const QByteArray start = sprm2(0x501C, 5);   // (the number to start at, as a section states it with or without the restart)
        DocWriter w = simple(QStringLiteral("x\r"));
        w.sections = {{int(w.text.size()) + 1, start}};
        Document d;
        QVERIFY(read(w.build(), &d));
        QVERIFY(!d.lastSection.pageStart.has_value());
        DocWriter r = simple(QStringLiteral("x\r"));
        r.sections = {{int(r.text.size()) + 1, start + sprm1(0x3011, 1)}};
        Document e;
        QVERIFY(read(r.build(), &e));
        QCOMPARE(e.lastSection.pageStart.value_or(-1), 5);
    }

    void notesAreNumberedAsTheDocumentSays()
    {
        DocWriter w = simple(QStringLiteral("x\r"));
        w.footnoteFormat = 4;
        Document d;
        QVERIFY(read(w.build(), &d));
        bool found = false;
        for (const wml::RawChild &c : d.lastSection.extra)
            if (c.xml.contains("footnotePr") && c.xml.contains("lowerLetter")) found = true;
        QVERIFY(found);
    }

    void aPictureThatIsNotThereIsLeftOut()
    {
        DocWriter w = simple(QStringLiteral("a\x01""b\r"));
        w.chars = {{1, 2, sprm1(sprm::special, 1) + sprm4(sprm::picture, 5000)}};
        Document d;
        ReadNotes notes;
        QVERIFY(read(w.build(), &d, &notes));
        QCOMPARE(bodyTexts(d), QStringList{QStringLiteral("ab")});
        QVERIFY(notes.leftOut);
        QVERIFY(!notes.notes.isEmpty());
    }

    // ---- drawings

    void aTextBoxIsSetWhereTheFileSaysAndHoldsItsText()
    {
        DocWriter w = simple(QStringLiteral("Before\x08 after\r"));
        w.chars = {{6, 7, sprm1(sprm::special, 1)}};
        ShapeSpec box;
        box.cp = 6;
        box.x0 = 720; box.y0 = 360; box.x1 = 4320; box.y1 = 1440;
        box.bx = 1; box.by = 0; box.wr = 2;
        box.text = QStringLiteral("Boxed words\r");
        w.shapes = {box};
        Document d;
        QVERIFY(read(w.build(), &d));
        QCOMPARE(bodyTexts(d), QStringList{QStringLiteral("Before after")});
        const Inline *pict = nullptr;
        for (const Inline &in : d.body[0].para.content)
            if (in.kind == Inline::RunAtom && in.tag == QLatin1String("pict")) pict = &in;
        QVERIFY(pict);
        const QVector<wml::Shape> shapes = wml::readShapes(pict->raw);
        QCOMPARE(shapes.size(), qsizetype(1));
        const wml::Shape &s = shapes[0];
        QCOMPARE(s.kind, wml::Shape::TextBox);
        QVERIFY(s.floating);
        QCOMPARE(s.left, 36.0);
        QCOMPARE(s.top, 18.0);
        QCOMPARE(s.width, 180.0);
        QCOMPARE(s.height, 54.0);
        QCOMPARE(s.hRel, wml::Shape::Page);
        QCOMPARE(s.vRel, wml::Shape::Margin);
        QCOMPARE(s.wrap, wml::Shape::Square);
        QVERIFY(!s.behind);
        QVERIFY(QString::fromUtf8(s.textBox).contains(QLatin1String("Boxed words")));
        // (The mark that ends the box's text is not a paragraph of it.)
        QCOMPARE(QString::fromUtf8(s.textBox).count(QLatin1String("<w:p>")), 1);
    }

    void aPictureShapeBehindTheText()
    {
        DocWriter w = simple(QStringLiteral("x\x08y\r"));
        w.chars = {{1, 2, sprm1(sprm::special, 1)}};
        ShapeSpec pic;
        pic.spid = 1030;
        pic.type = 75;
        pic.cp = 1;
        pic.x0 = 0; pic.y0 = 0; pic.x1 = 2880; pic.y1 = 1440;
        pic.wr = 3;
        pic.below = true;
        pic.png = tinyPng(16, 8);
        w.shapes = {pic};
        Document d;
        QVERIFY(read(w.build(), &d));
        const Inline *pict = nullptr;
        for (const Inline &in : d.body[0].para.content)
            if (in.kind == Inline::RunAtom && in.tag == QLatin1String("pict")) pict = &in;
        QVERIFY(pict);
        const QVector<wml::Shape> shapes = wml::readShapes(pict->raw);
        QCOMPARE(shapes.size(), qsizetype(1));
        QCOMPARE(shapes[0].kind, wml::Shape::Picture);
        QVERIFY(shapes[0].floating);
        QVERIFY(shapes[0].behind);
        QCOMPARE(shapes[0].wrap, wml::Shape::WrapNone);
        QCOMPARE(shapes[0].width, 144.0);
        QCOMPARE(shapes[0].height, 72.0);
        QCOMPARE(shapes[0].hRel, wml::Shape::Column);
        QCOMPARE(shapes[0].vRel, wml::Shape::Paragraph);
        QVERIFY(!shapes[0].embed.isEmpty());
        bool media = false;
        for (const wml::KeptPart &p : d.parts)
            if (p.name.startsWith(QLatin1String("/word/media/")) && p.data.startsWith(QByteArray("\x89PNG"))) media = true;
        QVERIFY(media);
    }

    void shapesOfTheHeadersAreInTheHeaders()
    {
        DocWriter w = simple(QStringLiteral("body\r"));
        w.headerStories = {QString(), QString(), QString(), QString(), QString(), QString(), QString(), QStringLiteral("\x08\r")};
        const int headerStart = int(w.text.size());
        w.chars = {{headerStart, headerStart + 1, sprm1(sprm::special, 1)}};
        ShapeSpec box;
        box.header = true;
        box.cp = 0;
        box.x0 = -720; box.y0 = -240; box.x1 = 7200; box.y1 = 600;
        box.text = QStringLiteral("Letterhead\r");
        w.shapes = {box};
        Document d;
        QVERIFY(read(w.build(), &d));
        const wml::HeaderRefs refs = wml::headerRefsOf(d.lastSection);
        QVERIFY(!refs.header[0].isEmpty());
        QString headerPart;
        for (const auto &r : d.mainRelationships)
            if (r.id == refs.header[0]) headerPart = r.target;
        const wml::KeptPart *p = part(d, headerPart);
        QVERIFY(p);
        QVERIFY(QString::fromUtf8(p->data).contains(QLatin1String("Letterhead")));
        QVERIFY(QString::fromUtf8(p->data).contains(QLatin1String("margin-left:-36pt")));
    }

    void theMembersOfAGroupAreScaledIntoTheGroup()
    {
        DocWriter w = simple(QStringLiteral("g\x08\r"));
        w.chars = {{1, 2, sprm1(sprm::special, 1)}};
        ShapeSpec group;
        group.spid = 1040;
        group.type = 0;
        group.cp = 1;
        group.x0 = 1000; group.y0 = 2000; group.x1 = 5000; group.y1 = 4000;   // 4000 by 2000 twips among the text
        group.groupLeft = 0; group.groupTop = 0; group.groupRight = 1000; group.groupBottom = 1000;   // its members' coordinates
        ShapeSpec member;
        member.spid = 1041;
        member.type = 1;
        member.group = 0;
        member.memberLeft = 250; member.memberTop = 500; member.memberRight = 750; member.memberBottom = 1000;
        member.fill = 0x0000FF;
        member.line = 0x000000;
        w.shapes = {group, member};
        Document d;
        QVERIFY(read(w.build(), &d));
        const Inline *pict = nullptr;
        for (const Inline &in : d.body[0].para.content)
            if (in.kind == Inline::RunAtom && in.tag == QLatin1String("pict")) pict = &in;
        QVERIFY(pict);
        const QVector<wml::Shape> shapes = wml::readShapes(pict->raw);
        QCOMPARE(shapes.size(), qsizetype(1));
        QCOMPARE(shapes[0].kind, wml::Shape::Rect);
        // A quarter of the way across 4000 twips from 1000 (to 2000), half way down 2000 from 2000 (to 3000): 50 by 50 points to 100 wide... as twips/20.
        QCOMPARE(shapes[0].left, 100.0);
        QCOMPARE(shapes[0].top, 150.0);
        QCOMPARE(shapes[0].width, 100.0);
        QCOMPARE(shapes[0].height, 50.0);
        QVERIFY(shapes[0].filled);
        QCOMPARE(shapes[0].fillColor, QStringLiteral("FF0000"));
    }

    void aShapeThatTheDrawingDoesNotHoldIsLeftOut()
    {
        DocWriter w = simple(QStringLiteral("x\x08y\r"));
        w.chars = {{1, 2, sprm1(sprm::special, 1)}};
        ShapeSpec box;
        box.cp = 1;
        box.x1 = 1440; box.y1 = 720;
        box.text = QStringLiteral("t\r");
        w.shapes = {box};
        QByteArray file = w.build();
        // (The shape's record is made another shape's: the place the file gives has no shape in the drawing.)
        jp::cfb::File f;
        QVERIFY(jp::cfb::read(file, &f));
        QByteArray t = f.stream(QStringLiteral("1Table"));
        const int at = t.indexOf(QByteArray::fromHex("01040000"));   // (the box's id, 1025, in the table of places)
        QVERIFY(at >= 0);
        t[at] = char(0x77);
        QVERIFY(f.setStream(QStringLiteral("1Table"), t));
        Document d;
        ReadNotes notes;
        QVERIFY(read(jp::cfb::write(f), &d, &notes));
        QCOMPARE(bodyTexts(d), QStringList{QStringLiteral("xy")});
        QVERIFY(notes.leftOut);
    }

    // ---- encrypted documents

    void knownAnswerStreamsFromASeparateScriptAreDeciphered()
    {
        const QString password = QString::fromUtf8("Pa\xc3\x9fw\xc3\xb6rd1");
        struct Form {
            const char *name, *wd, *table, *data;
            int description;
        };
        for (const Form &form : {Form{"RC4 40", kDocKatRC440_wd, kDocKatRC440_table, kDocKatRC440_data, 52}, Form{"CryptoAPI", kDocKatCRYPTOAPI_wd, kDocKatCRYPTOAPI_table, kDocKatCRYPTOAPI_data, 198}}) {
            const QByteArray file = compoundOf(QByteArray::fromBase64(form.wd), QByteArray::fromBase64(form.table), QByteArray::fromBase64(form.data));
            QVERIFY2(doc::docEncrypted(file), form.name);
            QByteArray out;
            QCOMPARE(doc::decryptDoc(file, QStringLiteral("not it"), 1 << 30, false, &out), CryptResult::WrongPassword);
            QVERIFY(out.isEmpty());   // (nothing is given for a wrong password)
            QCOMPARE(doc::decryptDoc(file, QString(), 1 << 30, false, &out), CryptResult::WrongPassword);
            QCOMPARE(doc::decryptDoc(file, password, 1 << 30, true, &out), CryptResult::Ok);
            QVERIFY(out.isEmpty());
            QCOMPARE(doc::decryptDoc(file, password, 1 << 30, false, &out), CryptResult::Ok);
            QVERIFY(!doc::docEncrypted(out));
            jp::cfb::File f;
            QVERIFY(jp::cfb::read(out, &f));
            const QByteArray wd = f.stream(QStringLiteral("WordDocument")), table = f.stream(QStringLiteral("1Table")), data = f.stream(QStringLiteral("Data"));
            // (a document stream's first 68 bytes were never enciphered; past them, and past the block of 512, the plain pattern is back)
            QCOMPARE(wd.size(), 1100);
            QCOMPARE(wd.mid(68), pattern(1100).mid(68));
            QCOMPARE(table.mid(form.description), pattern(900));
            QCOMPARE(data, pattern(700));
            QVERIFY2(!(quint16(quint8(wd[10]) | quint8(wd[11]) << 8) & 0x0100), form.name);
        }
    }

    void aKnownAnswerObfuscatedDocumentFromASeparateScriptIsDeciphered()
    {
        // (the password has the euro sign and a curly quote, which the Western Windows coding gives bytes of their own)
        const QString password = QString::fromUtf8("\xe2\x82\xac" "uro \xe2\x80\x99s");
        const QByteArray file = compoundOf(QByteArray::fromBase64(kDocKatXORwd), QByteArray::fromBase64(kDocKatXORtable), QByteArray::fromBase64(kDocKatXORdata));
        QVERIFY(doc::docEncrypted(file));
        QByteArray out;
        QCOMPARE(doc::decryptDoc(file, QStringLiteral("euro 's"), 1 << 30, false, &out), CryptResult::WrongPassword);
        QCOMPARE(doc::decryptDoc(file, QString::fromUtf8("\xc2\x80uro \xe2\x80\x99s"), 1 << 30, false, &out), CryptResult::WrongPassword);   // (not the euro sign, the control character before it)
        QVERIFY(out.isEmpty());
        QCOMPARE(doc::decryptDoc(file, password, 1 << 30, false, &out), CryptResult::Ok);
        jp::cfb::File f;
        QVERIFY(jp::cfb::read(out, &f));
        QCOMPARE(f.stream(QStringLiteral("WordDocument")).mid(68), pattern(1100).mid(68));
        QCOMPARE(f.stream(QStringLiteral("1Table")), pattern(900));
        QCOMPARE(f.stream(QStringLiteral("Data")), pattern(700));
    }

    void aCryptoApiHeaderThatIsNotTheRc4KindIsNotRead()
    {
        const QString password = QString::fromUtf8("Pa\xc3\x9fw\xc3\xb6rd1");
        const QByteArray file = compoundOf(QByteArray::fromBase64(kDocKatCRYPTOAPI_wd), QByteArray::fromBase64(kDocKatCRYPTOAPI_table), QByteArray::fromBase64(kDocKatCRYPTOAPI_data));
        jp::cfb::File f;
        QVERIFY(jp::cfb::read(file, &f));
        QByteArray out;
        for (const quint8 flags : {quint8(0x24), quint8(0x00)}) {   // (the AES kind; not made by the provider)
            QByteArray table = f.stream(QStringLiteral("1Table"));
            table[12] = char(flags);   // (the header's own flags: after the version, the info flags, and the size)
            jp::cfb::File g = f;
            g.setStream(QStringLiteral("1Table"), table);
            QCOMPARE(doc::decryptDoc(jp::cfb::write(g), password, 1 << 30, false, &out), CryptResult::Unsupported);
            QVERIFY(out.isEmpty());
        }
        QCOMPARE(doc::decryptDoc(file, password, 1 << 30, false, &out), CryptResult::Ok);
    }

    void aProtectedDocumentOpensWithItsPasswordInEveryForm()
    {
        const QString password = QString::fromUtf8("p\xc3\xa4ss word");
        const QStringList forms = {QStringLiteral("RC4, 40 bits"), QStringLiteral("CryptoAPI"), QStringLiteral("obfuscation")};
        for (int form = 0; form < 3; ++form) {
            DocWriter w = simple(QStringLiteral("Secret plans\r") + QString(600, QLatin1Char('x')) + QStringLiteral("\rEnd\r"));
            const QByteArray file = w.buildProtected(form == 2 ? QStringLiteral("pass word") : password, form);
            QVERIFY2(!file.isEmpty(), qPrintable(forms[form]));
            // as it is, the file does not read, and says why
            Document d;
            QString why;
            QVERIFY2(!read(file, &d, nullptr, &why), qPrintable(forms[form]));
            QVERIFY2(why.contains(QLatin1String("password")), qPrintable(why));
            QVERIFY(doc::docEncrypted(file));
            QByteArray plain;
            QCOMPARE(doc::decryptDoc(file, QStringLiteral("wrong"), 1 << 30, false, &plain), CryptResult::WrongPassword);
            QVERIFY(plain.isEmpty());
            QCOMPARE(doc::decryptDoc(file, form == 2 ? QStringLiteral("pass word") : password, 1 << 30, false, &plain), CryptResult::Ok);
            QVERIFY2(read(plain, &d, nullptr, &why), qPrintable(forms[form] + why));
            const QStringList texts = bodyTexts(d);
            QCOMPARE(texts.size(), 3);
            QCOMPARE(texts[0], QStringLiteral("Secret plans"));
            QCOMPARE(texts[1].size(), 600);
            QCOMPARE(texts[2], QStringLiteral("End"));
        }
    }

    void aProtectedDocumentIsRefusedWhenItsHeaderOrSizesAreHostile()
    {
        DocWriter w = simple(QStringLiteral("x\r"));
        const QByteArray file = w.buildProtected(QStringLiteral("pw"), 1);
        QByteArray out;
        QCOMPARE(doc::decryptDoc(file, QStringLiteral("pw"), file.size() - 1, false, &out), CryptResult::TooLarge);
        jp::cfb::File f;
        QVERIFY(jp::cfb::read(file, &f));
        // the description of the protection said to be larger than the table stream, or than anything the form has, or too small to hold what it must
        for (const quint32 claim : {quint32(1) << 30, quint32(5000), quint32(10), quint32(0)}) {
            QByteArray wd = f.stream(QStringLiteral("WordDocument"));
            for (int i = 0; i < 4; ++i) wd[14 + i] = char((claim >> (8 * i)) & 0xFF);
            jp::cfb::File g = f;
            g.setStream(QStringLiteral("WordDocument"), wd);
            QCOMPARE(doc::decryptDoc(jp::cfb::write(g), QStringLiteral("pw"), 1 << 30, false, &out), CryptResult::Damaged);
            QVERIFY(out.isEmpty());
        }
        // a header from before 1997 is not taken for a protected document (the reader says what it says of such files)
        QByteArray wd = f.stream(QStringLiteral("WordDocument"));
        wd[2] = char(0x65);
        wd[3] = 0;
        jp::cfb::File g = f;
        g.setStream(QStringLiteral("WordDocument"), wd);
        QVERIFY(!doc::docEncrypted(jp::cfb::write(g)));
        // stopped while the blocks are deciphered
        DocWriter big = simple(QString(30000, QLatin1Char('y')) + QStringLiteral("\r"));
        int asked = 0;
        QCOMPARE(doc::decryptDoc(big.buildProtected(QStringLiteral("pw"), 0), QStringLiteral("pw"), 1 << 30, false, &out, [&asked] { ++asked; return true; }), CryptResult::Cancelled);
        QVERIFY(asked > 0);
        QVERIFY(out.isEmpty());
        // not a compound file, and a compound file with no document in it
        QVERIFY(!doc::docEncrypted(QByteArray("plain text")));
        QCOMPARE(doc::decryptDoc(QByteArray("plain text"), QStringLiteral("pw"), 1 << 30, false, &out), CryptResult::Damaged);
    }


    // ---- real files of other programs, when the collections are there (never copied into the repository; without JO_CORPORA the rows skip)

    void realProtectedDocumentsOpen_data()
    {
        QTest::addColumn<QString>("path");
        QTest::addColumn<QString>("password");
        const QString poi = qEnvironmentVariable("JO_CORPORA") + QStringLiteral("/poi/test-data/document/");
        QTest::newRow("RC4, 40 bits") << poi + QStringLiteral("password_tika_binaryrc4.doc") << QStringLiteral("tika");
        QTest::newRow("CryptoAPI") << poi + QStringLiteral("password_password_cryptoapi.doc") << QStringLiteral("password");
    }

    void realProtectedDocumentsOpen()
    {
        QFETCH(QString, path);
        QFETCH(QString, password);
        QByteArray file;
        if (!readWholeFile(path, &file)) QSKIP("the collection is not here");
        QVERIFY(doc::docEncrypted(file));
        QByteArray plain;
        QCOMPARE(doc::decryptDoc(file, QStringLiteral("wrong"), 1 << 30, false, &plain), CryptResult::WrongPassword);
        QCOMPARE(doc::decryptDoc(file, password, 1 << 30, false, &plain), CryptResult::Ok);
        Document d;
        QString why;
        QVERIFY2(read(plain, &d, nullptr, &why), qPrintable(why));
        QVERIFY(!bodyTexts(d).join(QLatin1Char(' ')).trimmed().isEmpty());
    }

    // ---- files that are not read

    void refusals()
    {
        Document d;
        QString why;
        QVERIFY(!read(QByteArray("this is not a compound file at all"), &d, nullptr, &why));
        QVERIFY(!why.isEmpty());
        DocWriter pw = simple(QStringLiteral("secret\r"));
        pw.password = true;
        why.clear();
        QVERIFY(!read(pw.build(), &d, nullptr, &why));
        QVERIFY2(why.contains(QLatin1String("password")), qPrintable(why));
        DocWriter ow = simple(QStringLiteral("old\r"));
        ow.oldVersion = true;
        why.clear();
        QVERIFY(!read(ow.build(), &d, nullptr, &why));
        QVERIFY2(why.contains(QLatin1String("1997")), qPrintable(why));
        // Older still, and not compound files at all: the 1991 program's, and the 1992 Macintosh program's. Not "damaged": from before 1997.
        for (const QByteArray &start : {QByteArray::fromHex("dba52d00"), QByteArray::fromHex("fe370023"), QByteArray::fromHex("31be0000")}) {
            why.clear();
            QVERIFY(!read(start + QByteArray(600, '\0'), &d, nullptr, &why));
            QVERIFY2(why.contains(QLatin1String("1997")), qPrintable(start.toHex() + ": " + why));
        }
        why.clear();
        QVERIFY(!read(QByteArray(), &d, nullptr, &why));
    }

    void looksLikeACompoundFile()
    {
        QVERIFY(doc::looksLikeCompoundFile(simple(QStringLiteral("x\r")).build()));
        QVERIFY(!doc::looksLikeCompoundFile(QByteArray("PK\x03\x04 and more bytes")));
        QVERIFY(!doc::looksLikeCompoundFile(QByteArray()));
    }

    // ---- hostile files

    void aTextLongerThanTheFileSaysIsCut()
    {
        QByteArray file = simple(QStringLiteral("short\r")).build();
        Document d;
        ReadNotes notes;
        // (Header changed by hand: the main text claims two billion characters.)
        jp::cfb::File f;
        QVERIFY(jp::cfb::read(file, &f));
        QByteArray wd = f.stream(QStringLiteral("WordDocument"));
        wd[76] = char(0xFF);
        wd[77] = char(0xFF);
        wd[78] = char(0xFF);
        wd[79] = char(0x7F);
        QVERIFY(f.setStream(QStringLiteral("WordDocument"), wd));
        QString error;
        QVERIFY2(read(jp::cfb::write(f), &d, &notes, &error), qPrintable(error));
        QVERIFY2(doc::readWorkForTest().charsConverted <= 100, qPrintable(QString::number(doc::readWorkForTest().charsConverted)));   // (the text the file holds, not the length it claims)
        // (What the file holds is read; the length it claims is not believed, and the cut is said.)
        QCOMPARE(bodyTexts(d), QStringList{QStringLiteral("short")});
        QVERIFY(notes.leftOut);
    }

    void tooManyRowsAreCutAndSaid()
    {
        // More rows than the .docx reader lays out: the rest are left out, and the time stays short.
        const int rows = wml::kMaxTableRows + 300;
        QString text;
        DocWriter w;
        const QByteArray in = sprm1(sprm::inTable, 1) + sprm4(sprm::tableDepth, 1);
        const QByteArray row = in + sprm1(sprm::endsRow, 1) + tableRow({0, 1000});
        int at = 0;
        for (int i = 0; i < rows; ++i) {
            text += QStringLiteral("x\a\a");
            w.paras.push_back({at, at + 2, 0, in});
            w.paras.push_back({at + 2, at + 3, 0, row});
            at += 3;
        }
        text += QStringLiteral("end\r");
        w.text = text;
        Document d;
        ReadNotes notes;
        QVERIFY(read(w.build(), &d, &notes));
        QVERIFY2(doc::readWorkForTest().charsConverted <= text.size() && doc::readWorkForTest().outputElements <= doc::kMaxOutputElements, qPrintable(QString::number(doc::readWorkForTest().outputElements)));
        int count = 0;
        for (const Block &b : d.body)
            if (b.isRaw && b.tag == QLatin1String("tr") && b.role == Inline::Open) ++count;
        QVERIFY(count <= wml::kMaxTableRows);
        QVERIFY(notes.leftOut);
    }

    void manySectionsAreBounded()
    {
        const int sections = doc::kMaxSections + 500;
        QString text;
        DocWriter w;
        for (int i = 0; i < sections; ++i) {
            text += QLatin1Char('\f');
            w.paras.push_back({i, i + 1, 0, QByteArray()});
            w.sections.push_back({i + 1, QByteArray()});
        }
        w.text = text;
        Document d;
        QVERIFY(read(w.build(), &d));
        QVERIFY2(doc::readWorkForTest().charsConverted <= sections && doc::readWorkForTest().outputElements <= doc::kMaxOutputElements, qPrintable(QString::number(doc::readWorkForTest().outputElements)));
        int breaks = 0;
        for (const Block &b : d.body)
            if (!b.isRaw && b.para.props.section) ++breaks;
        QVERIFY(breaks <= doc::kMaxSections);
    }

    void aPictureManyCharactersPointToIsReadOnce()
    {
        DocWriter w;
        const int at = w.addPicture(tinyPng(32, 32), 1440, 1440);
        QString text;
        const int refs = 3000;
        for (int i = 0; i < refs; ++i) {
            text += QLatin1Char('\x01');
            w.chars.push_back({i, i + 1, sprm1(sprm::special, 1) + sprm4(sprm::picture, quint32(at))});
        }
        text += QLatin1Char('\r');
        w.text = text;
        Document d;
        QVERIFY(read(w.build(), &d));
        QCOMPARE(doc::readWorkForTest().picturesRead, qint64(1));   // (3,000 characters point to it: it is read once)
        int media = 0;
        for (const wml::KeptPart &p : d.parts)
            if (p.name.startsWith(QLatin1String("/word/media/"))) ++media;
        QCOMPARE(media, 1);
    }

    void notesThatOverlapAreLeftOut()
    {
        // The table of the notes' text says the second begins at 255, past the end of the notes' text, and the first has to end there: the notes follow one
        // another, so this is not read, rather than read again for each.
        DocWriter w = simple(QStringLiteral("a\x02""b\x02""c\r"));
        w.chars = {{1, 2, sprm1(sprm::special, 1)}, {3, 4, sprm1(sprm::special, 1)}};
        w.footnotes = {QStringLiteral("\x02 one\r"), QStringLiteral("\x02 two\r")};
        w.footnoteRefs = {1, 3};
        // (As the file should have it, the notes are read.)
        {
            Document d;
            QVERIFY(read(w.build(), &d));
            QVERIFY(part(d, QStringLiteral("/word/footnotes.xml")) != nullptr);
        }
        w.footnoteStarts = {0, 255};
        Document d;
        QVERIFY(read(w.build(), &d));
        QCOMPARE(bodyTexts(d), QStringList{QStringLiteral("abc")});
        QVERIFY(part(d, QStringLiteral("/word/footnotes.xml")) == nullptr || !QString::fromUtf8(part(d, QStringLiteral("/word/footnotes.xml"))->data).contains(QLatin1String("one")));
    }

    void aLongChainOfStylesIsCutShort()
    {
        DocWriter w = simple(QStringLiteral("x\r"));
        QVector<StyleSpec> st;
        for (int i = 0; i < 300; ++i) {
            StyleSpec s;
            s.name = QStringLiteral("S%1").arg(i);
            s.sti = i == 0 ? 0 : 0xFFE;
            s.base = i == 0 ? 0xFFF : i - 1;
            s.character = sprm1(sprm::bold, 0x81);
            st.push_back(s);
        }
        w.styles = st;
        w.paras = {{0, 2, 299, QByteArray()}};
        Document d;
        QVERIFY(read(w.build(), &d));
        QVERIFY2(doc::readWorkForTest().styleSteps <= 300 * (doc::kMaxStyleDepth + 2), qPrintable(QString::number(doc::readWorkForTest().styleSteps)));
        QCOMPARE(bodyTexts(d), QStringList{"x"});
        // (The style the paragraph has is still read, and its chain no longer than the limit.)
        int depth = 0;
        for (const wml::Style *s = d.styles.find(QStringLiteral("S299")); s && depth < 1000; s = s->basedOn.isEmpty() ? nullptr : d.styles.find(s->basedOn)) ++depth;
        QVERIFY(depth <= doc::kMaxStyleDepth + 2);
    }

    // ---- files made to cost a great deal (each must be read in a couple of seconds)

    void aGroupAnchoredOftenIsNotWalkedOverAndOver()
    {
        // 20,000 members the reader shows nothing of, in one group that 1,000 anchors point to: each anchor walked the group again (20 million visits).
        const int members = 20000, anchors = 1000;
        DocWriter w = simple(QString(anchors, QLatin1Char('\x08')) + QLatin1Char('\r'));
        w.chars = {{0, anchors, sprm1(sprm::special, 1)}};
        ShapeSpec group;
        group.spid = 2000;
        group.type = 0;
        group.cp = 0;
        for (int i = 1; i < anchors; ++i) group.moreAnchors.push_back(i);
        w.shapes.push_back(group);
        for (int i = 0; i < members; ++i) {
            ShapeSpec m;
            m.spid = 3000 + i;
            m.type = 0;
            m.group = 0;
            m.memberLeft = 0; m.memberTop = 0; m.memberRight = 10; m.memberBottom = 10;
            w.shapes.push_back(m);
        }
        const QByteArray file = w.build();
        Document d;
        ReadNotes notes;
        QVERIFY(read(file, &d, &notes));
        // (Walks have a budget of their own: 20,000 members for each of 1,000 anchors would be 20 million visits.)
        QVERIFY2(doc::readWorkForTest().shapeVisits <= doc::kMaxShapeVisits + 2 * members, qPrintable(QString::number(doc::readWorkForTest().shapeVisits)));
        QVERIFY(notes.leftOut);
    }

    void aShapeNumberThatDoublesPastTheLimitDoesNotMeetAnother()
    {
        // The ids 0x40000000 and 0xC0000000 doubled (the way text boxes already shown are kept) are the same number in 32 bits: the second box lost its text.
        DocWriter w = simple(QStringLiteral("a\x08""b\x08\r"));
        w.chars = {{1, 2, sprm1(sprm::special, 1)}, {3, 4, sprm1(sprm::special, 1)}};
        ShapeSpec one, two;
        one.spid = 0x40000000;
        one.cp = 1;
        one.x1 = 1440; one.y1 = 720;
        one.text = QStringLiteral("first box\r");
        two.spid = qint32(0xC0000000u);
        two.cp = 3;
        two.x1 = 1440; two.y1 = 720;
        two.text = QStringLiteral("second box\r");
        w.shapes = {one, two};
        Document d;
        QVERIFY(read(w.build(), &d));
        QStringList boxes;
        for (const Inline &in : d.body[0].para.content)
            if (in.kind == Inline::RunAtom && in.tag == QLatin1String("pict"))
                for (const wml::Shape &s : wml::readShapes(in.raw)) boxes << QString::fromUtf8(s.textBox);
        QCOMPARE(boxes.size(), qsizetype(2));
        QVERIFY(boxes[0].contains(QLatin1String("first box")));
        QVERIFY(boxes[1].contains(QLatin1String("second box")));
    }

    void pieceTablesThatShareTextCannotMakeMoreXmlNodesThanTheReaderTakes()
    {
        // 90 characters of text, each its own run (bold, then not), listed 5,000 times over: 450,000 runs from a file of a few kilobytes. The XML of that
        // has more nodes than the .docx reader takes in a part, and the document was refused after all the work of making it.
        const QString line = QStringLiteral("abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz");
        DocWriter w;
        w.text = line.left(89) + QLatin1Char('\r');
        for (int i = 0; i < 89; ++i) w.chars.push_back({i, i + 1, sprm1(sprm::bold, i & 1)});
        w.repeatText = 5000;
        const QByteArray file = w.build();
        QVERIFY(file.size() < 200000);
        Document d;
        ReadNotes notes;
        QString error;
        QVERIFY2(read(file, &d, &notes, &error), qPrintable(error));
        QVERIFY2(doc::readWorkForTest().outputElements <= doc::kMaxOutputElements + 100 && doc::readWorkForTest().outputBytes <= doc::kMaxOutputBytes + (1 << 20), qPrintable(QString::number(doc::readWorkForTest().outputElements)));
        QVERIFY2(doc::readWorkForTest().charsConverted <= 5000 * 90, qPrintable(QString::number(doc::readWorkForTest().charsConverted)));
        QVERIFY(notes.leftOut);
        QVERIFY(notes.notes.join(QLatin1Char(' ')).contains(QLatin1String("longer than this program can open")));
        // (What came before the cut is there, whole paragraphs of it.)
        const QStringList texts = bodyTexts(d);
        QVERIFY(texts.size() > 100 && texts.size() < 5000);
        QCOMPARE(texts[0], w.text.left(89));
    }

    void pieceTablesThatShareTextCannotMakeMoreXmlBytesThanTheReaderTakes()
    {
        // 1,000 ampersands (five bytes of XML each) listed 15,000 times over: 75 MB of XML from a file of a few kilobytes, past what the reader takes of a part.
        DocWriter w;
        w.text = QString(1000, QLatin1Char('&')) + QLatin1Char('\r');
        w.repeatText = 15000;
        const QByteArray file = w.build();
        QVERIFY(file.size() < 200000);
        Document d;
        ReadNotes notes;
        QString error;
        QVERIFY2(read(file, &d, &notes, &error), qPrintable(error));
        QVERIFY2(doc::readWorkForTest().outputBytes <= doc::kMaxOutputBytes + (1 << 20), qPrintable(QString::number(doc::readWorkForTest().outputBytes)));
        QVERIFY2(doc::readWorkForTest().charsConverted <= 15000 * 1001 / 10, qPrintable(QString::number(doc::readWorkForTest().charsConverted)));
        QVERIFY(notes.leftOut);
        QVERIFY(notes.notes.join(QLatin1Char(' ')).contains(QLatin1String("longer than this program can open")));
        const QStringList texts = bodyTexts(d);
        QVERIFY(texts.size() > 1000 && texts.size() < 15000);
        QCOMPARE(texts[0], QString(1000, QLatin1Char('&')));
    }

    void aSingleParagraphLongerThanTheOutputMayBeIsCutAndSaid()
    {
        // The same, as one paragraph (no mark ends it): it is cut in the middle, and the cut is said.
        DocWriter w;
        w.text = QString(1000, QLatin1Char('&'));
        w.repeatText = 15000;
        const QByteArray file = w.build();
        Document d;
        ReadNotes notes;
        QString error;
        QVERIFY2(read(file, &d, &notes, &error), qPrintable(error));
        // (One paragraph is cut in the middle of a piece: a few megabytes past the limit at most.)
        QVERIFY2(doc::readWorkForTest().outputBytes <= doc::kMaxOutputBytes + (8 << 20), qPrintable(QString::number(doc::readWorkForTest().outputBytes)));
        QVERIFY2(doc::readWorkForTest().charsConverted <= 15000 * 1000 / 10, qPrintable(QString::number(doc::readWorkForTest().charsConverted)));
        QVERIFY(notes.leftOut);
        QVERIFY(notes.notes.join(QLatin1Char(' ')).contains(QLatin1String("longer than this program can open")));
        const QStringList texts = bodyTexts(d);
        QVERIFY(!texts.isEmpty());
        QVERIFY(texts[0].size() > 1000000 && texts[0].size() < 15000000);
    }

    void picturesTogetherAreHeldToALimit()
    {
        // Four metafiles that each make 30 MB once inflated, from a few kilobytes of file apiece: the pictures together could be 120 MB, or (the limit then)
        // 384 MB, held while the package was made, written, and read again.
        DocWriter w;
        QString text;
        const int pictures = 4;
        for (int i = 0; i < pictures; ++i) {
            const int at = w.addMetafile(30 << 20, 1440, 1440);
            w.chars.push_back({i, i + 1, sprm1(sprm::special, 1) + sprm4(sprm::picture, quint32(at))});
            text += QLatin1Char('\x01');
        }
        w.text = text + QLatin1Char('\r');
        const QByteArray file = w.build();
        QVERIFY(file.size() < 1000000);
        Document d;
        ReadNotes notes;
        QString error;
        QVERIFY2(read(file, &d, &notes, &error), qPrintable(error));
        QVERIFY2(doc::readWorkForTest().picturesRead == pictures && doc::readWorkForTest().outputBytes <= doc::kMaxOutputBytes + (1 << 20), qPrintable(QString::number(doc::readWorkForTest().picturesRead)));
        qint64 bytes = 0;
        int media = 0;
        for (const wml::KeptPart &p : d.parts)
            if (p.name.startsWith(QLatin1String("/word/media/"))) {
                ++media;
                bytes += p.data.size();
            }
        QVERIFY(media >= 1 && media < pictures);
        QVERIFY(bytes <= doc::kMaxPictureTotal);
        QVERIFY(notes.leftOut);
        QVERIFY(notes.notes.join(QLatin1Char(' ')).contains(QLatin1String("pictures could not be shown")));
    }

    void changedBytesNeverCrashOrHang()
    {
        // A document with most of what the reader reads, with bytes of it changed at random (the same changes each run), and cut short.
        DocWriter w = simple(QStringLiteral("Hello \x13 PAGE \x14""1\x15 world\a\a\rcell\a\a\r\x01 end\rnote\x02\r"));
        w.fonts = {QStringLiteral("Times New Roman"), QStringLiteral("Arial")};
        StyleSpec normal;
        normal.name = QStringLiteral("Normal");
        normal.sti = 0;
        StyleSpec head;
        head.name = QStringLiteral("heading 1");
        head.sti = 1;
        head.base = 0;
        head.character = sprm1(sprm::bold, 1);
        w.styles = {normal, head};
        ListLevelSpec dec;
        dec.text = QString(QChar(0)) + QLatin1Char('.');
        w.lists = {{dec}};
        w.footnotes = {QStringLiteral("\x02 note\r")};
        w.footnoteRefs = {10};
        w.headerStories = {QString(), QString(), QString(), QString(), QString(), QString(), QStringLiteral("head\r"), QString(), QString(), QString(), QString(), QString()};
        const int pic = w.addPicture(tinyPng(), 1440, 720);
        w.chars = {{0, 5, sprm1(sprm::bold, 1)}, {30, 31, sprm1(sprm::special, 1) + sprm4(sprm::picture, quint32(pic))}};
        const QByteArray in = sprm1(sprm::inTable, 1) + sprm4(sprm::tableDepth, 1);
        w.paras = {{0, 24, 1, sprm2(sprm::listInstance, 1)}, {24, 26, 0, in + sprm1(sprm::endsRow, 1) + tableRow({0, 1000, 2000})}};
        w.sections = {{40, sprm2(sprm::pageWidth, 12240)}, {int(w.text.size()) + 1, QByteArray()}};
        const QByteArray good = w.build();
        Document ok;
        QVERIFY(read(good, &ok));
        QRandomGenerator rng(20261016);
        QElapsedTimer total;
        total.start();
        int read1 = 0;
        // (The changes are made where the file holds something: the zeros that fill out its sectors would take a change and do nothing.)
        const QVector<int> live = liveBytes(good);
        QVERIFY(live.size() > 1000);
        for (int round = 0; round < 400; ++round) {
            QByteArray bad = good;
            const int changes = 1 + int(rng.bounded(8));
            for (int i = 0; i < changes; ++i) {
                const int at = live[int(rng.bounded(quint32(live.size())))];
                switch (rng.bounded(3u)) {
                case 0: bad[at] = char(rng.bounded(256u)); break;
                case 1: bad[at] = char(0xFF); break;
                default: bad[at] = char(bad[at] ^ (1 << rng.bounded(8u))); break;
                }
            }
            if (round % 7 == 0) bad.truncate(int(rng.bounded(quint32(bad.size()))));
            Document d;
            // (The clock stays as a watchdog for a hang: three seconds is hundreds of times what a round takes.)
            QElapsedTimer one;
            one.start();
            if (read(bad, &d)) ++read1;
            // (Counted: whatever the bytes say, the work stays inside the reader's own limits.)
            const doc::ReadWork work = doc::readWorkForTest();
            QVERIFY2(work.outputBytes <= doc::kMaxOutputBytes + (1 << 20) && work.outputElements <= doc::kMaxOutputElements + 100 && work.shapeVisits <= doc::kMaxShapeVisits + doc::kMaxShapes
                         && work.picturesRead <= doc::kMaxPictures && work.styleSteps <= 4096 * (doc::kMaxStyleDepth + 2),
                     qPrintable(QStringLiteral("round %1: %2 bytes, %3 elements, %4 shape visits, %5 pictures, %6 style steps").arg(round).arg(work.outputBytes).arg(work.outputElements).arg(work.shapeVisits).arg(work.picturesRead).arg(work.styleSteps)));
            QVERIFY2(one.elapsed() < 3000, qPrintable(QStringLiteral("round %1 took %2 ms").arg(round).arg(one.elapsed())));
        }
        QVERIFY(read1 > 0);
        QVERIFY(total.elapsed() < 120000);
    }

    // ---- Open and Repair

    void aSoundDocumentIsRepairedToItself()
    {
        QStringList paras, notes;
        const QByteArray good = repairableDoc(&paras);
        QCOMPARE(repairedParagraphs(good, &notes), paras);
        QVERIFY2(notes.isEmpty(), qPrintable(notes.join(QLatin1Char('|'))));
    }

    void aDocumentCutShortKeepsTheTextBeforeTheCut()
    {
        QStringList paras;
        const QByteArray good = repairableDoc(&paras);
        int before = 1 << 20;
        for (int percent : {90, 75, 60, 45}) {
            const QByteArray cut = good.left(good.size() * percent / 100);
            Document plain;
            QVERIFY(!read(cut, &plain));   // (the tables at the end that name the streams are gone)
            QStringList notes;
            QString why;
            QStringList got = repairedParagraphs(cut, &notes, &why);
            QVERIFY2(!got.isEmpty(), qPrintable(QString::number(percent) + QLatin1Char(' ') + why));
            QVERIFY(got.size() <= paras.size());   // (the text comes first in the stream, and the cut is in what follows it until 75 percent)
            if (percent == 45) QVERIFY(got.size() < paras.size());
            got.removeLast();   // (the last may be cut in the middle)
            QCOMPARE(got, paras.mid(0, got.size()));
            QVERIFY2(got.size() <= before, qPrintable(QString::number(percent)));   // (less of the file, less of the text)
            before = int(got.size());
            QVERIFY(notes.join(QLatin1Char(' ')).contains(QLatin1String("cut short")));
        }
        QVERIFY(before > 40);
        // cut before the document stream starts: nothing to keep, and said so
        QString why;
        QStringList notes;
        QVERIFY(doc::salvageDoc(good.left(1000), &notes, &why).isEmpty());
        QVERIFY(!why.isEmpty());
    }

    void aDocumentWhoseTableOfSectorsIsZeroedIsRepaired()
    {
        QStringList paras, notes;
        QByteArray bad = repairableDoc(&paras);
        zeroSector(bad, int(le32At(bad, 76)));
        Document plain;
        QVERIFY(!read(bad, &plain) || bodyTexts(plain) != paras);
        QCOMPARE(repairedParagraphs(bad, &notes), paras);
        QVERIFY2(notes.join(QLatin1Char(' ')).contains(QLatin1String("directory")), qPrintable(notes.join(QLatin1Char('|'))));
    }

    void aDocumentWhoseDirectoryIsZeroedIsFoundByItsRecords()
    {
        QStringList paras, notes;
        QByteArray bad = repairableDoc(&paras);
        Document plain;
        zeroSector(bad, int(le32At(bad, 48)));
        QVERIFY(!read(bad, &plain));
        QCOMPARE(repairedParagraphs(bad, &notes), paras);
        QVERIFY(notes.join(QLatin1Char(' ')).contains(QLatin1String("directory")));
    }

    void aChainOfSectorsThatLoopsIsFollowedOnce()
    {
        QStringList paras, notes;
        QByteArray bad = repairableDoc(&paras);
        const int wd = directoryEntryOf(bad, QStringLiteral("WordDocument"));
        QVERIFY(wd > 0);
        const quint32 start = le32At(bad, wd + 116);
        const int fat = (int(le32At(bad, 76)) + 1) * 512;
        QVERIFY(start + 6 < 128);
        setLe32(bad, fat + int(start + 5) * 4, start + 1);   // (a later sector leads back to the second)
        Document plain;
        QVERIFY(!read(bad, &plain, nullptr) || bodyTexts(plain) != paras);
        QCOMPARE(repairedParagraphs(bad, &notes), paras);
        QVERIFY2(notes.join(QLatin1Char(' ')).contains(QLatin1String("directory")), qPrintable(notes.join(QLatin1Char('|'))));   // (the chain was guessed from the directory)
        // the directory's own chain too
        QByteArray bad2 = repairableDoc(&paras);
        setLe32(bad2, (int(le32At(bad2, 76)) + 1) * 512 + int(le32At(bad2, 48)) * 4, le32At(bad2, 48));
        QCOMPARE(repairedParagraphs(bad2, &notes), paras);
    }

    void aDocumentWhoseTablesAreLostKeepsItsTextAsPlainParagraphs()
    {
        QStringList paras, notes;
        QByteArray bad = repairableDoc(&paras);
        // (the small streams, the tables among them, are in the container's mini stream: its sectors are zeroed)
        const int root = (int(le32At(bad, 48)) + 1) * 512;
        const int first = int(le32At(bad, root + 116)), sectors = int((le32At(bad, root + 120) + 511) / 512);
        QVERIFY(sectors > 0);
        for (int n = first; n < first + sectors; ++n) zeroSector(bad, n);
        QString why;
        QCOMPARE(repairedParagraphs(bad, &notes, &why), paras);
        QVERIFY(notes.join(QLatin1Char(' ')).contains(QLatin1String("plain paragraphs")));
    }

    void aDocumentProtectedWithAPasswordIsNotRepaired()
    {
        DocWriter w = simple(QStringLiteral("Secret\r"));
        QString why;
        QStringList notes;
        for (int form : {0, 1, 2}) {
            const QByteArray locked = w.buildProtected(QStringLiteral("pw"), form);
            QVERIFY(doc::salvageDoc(locked, &notes, &why).isEmpty());
            QVERIFY2(why.contains(QLatin1String("password")), qPrintable(why));
        }
    }

    void aScanChecksAtMostTheNumberOfPlacesItIsAllowed()
    {
        // 200,000 bytes in which every place looks like a start: the full check is called at most maxChecks times, on at most maxStreamBytes
        QByteArray junk(200000, 'x');
        int calls = 0;
        qsizetype biggest = 0;
        jp::office::CfbWish w;
        w.name = QStringLiteral("X");
        w.looksLike = [](QByteArrayView, const QMap<QString, QByteArray> &) { return true; };
        w.validBytes = [&](QByteArrayView b, const QMap<QString, QByteArray> &) -> qint64 {
            ++calls;
            biggest = std::max(biggest, b.size());
            return 0;
        };
        jp::office::CfbSalvageLimits lim;
        lim.maxChecks = 10;
        lim.maxStreamBytes = 5000;
        jp::office::CfbSalvage out;
        QString why;
        QVERIFY(!jp::office::salvageCompoundFile(junk, {w}, &out, &why, lim));
        QCOMPARE(calls, 10);
        QCOMPARE(biggest, qsizetype(5000));
        // a file over the limit is not looked through at all
        calls = 0;
        lim.maxFileBytes = 1000;
        QVERIFY(!jp::office::salvageCompoundFile(junk, {w}, &out, &why, lim));
        QCOMPARE(calls, 0);
        QVERIFY(!why.isEmpty());
        // more streams than are allowed to be asked for
        lim = jp::office::CfbSalvageLimits();
        lim.maxWishes = 1;
        QVERIFY(!jp::office::salvageCompoundFile(junk, {w, w}, &out, &why, lim));
        QCOMPARE(calls, 0);
    }

    void aStreamOverTheLimitIsNotHeld()
    {
        QStringList paras;
        const QByteArray good = repairableDoc(&paras);
        jp::office::CfbWish w;
        w.name = QStringLiteral("WordDocument");
        jp::office::CfbSalvageLimits lim;
        lim.maxStreamBytes = 4096;   // (the document stream is 30,000 bytes or so)
        jp::office::CfbSalvage out;
        QString why;
        QVERIFY(!jp::office::salvageCompoundFile(good, {w}, &out, &why, lim));
        QVERIFY(out.streams.isEmpty());
        QVERIFY(jp::office::salvageCompoundFile(good, {w}, &out, &why));
        QVERIFY(out.streams.value(QStringLiteral("WordDocument")).size() > 4096);
    }

    // The limits on what a hostile file makes the repair do.

    void aTableOfPiecesAfterManyModifiersIsLookedThroughOnlyAFewSteps()
    {
        QStringList paras, notes;
        jp::cfb::File f;
        QVERIFY(jp::cfb::read(repairableDoc(&paras), &f));
        // the table stream is made all of property modifiers (a 1, then a length, again and again), so each place the table of pieces could be would be walked to its end
        const QByteArray table(60000, '\x01');
        QVERIFY(f.setStream(QStringLiteral("1Table"), table));
        doc::salvageDoc(jp::cfb::write(f), &notes, nullptr);
        QVERIFY(doc::salvageWorkForTest().maxModifierSteps > 0 && doc::salvageWorkForTest().maxModifierSteps <= 16);
    }

    void theHeaderIsReadOnceWhateverTheNumberOfPlacesScanned()
    {
        QStringList paras, notes;
        QByteArray bad = repairableDoc(&paras);
        zeroSector(bad, int(le32At(bad, 48)));   // (the directory is gone: the streams are found by scanning)
        QCOMPARE(repairedParagraphs(bad, &notes), paras);
        QVERIFY(doc::salvageWorkForTest().fibReads <= 2);   // (the file has some 500 places to look at)
    }

private:
    // The table of pieces of a document of one piece, made wrong in one place (the document and its stream of tables are the file's own): `change` gets the table and where the piece table starts.
    void tableWrongIn(const std::function<void(QByteArray &, int, int)> &change)
    {
        QStringList paras, notes;
        jp::cfb::File f;
        QVERIFY(jp::cfb::read(repairableDoc(&paras), &f));
        const QByteArray wd = f.stream(QStringLiteral("WordDocument"));
        const int pairs = 34 + qFromLittleEndian<quint16>(wd.constData() + 32) * 2;
        const int cslw = qFromLittleEndian<quint16>(wd.constData() + pairs);
        const int clx = int(qFromLittleEndian<quint32>(wd.constData() + pairs + 2 + cslw * 4 + 2 + 33 * 8));
        QByteArray table = f.stream(QStringLiteral("1Table"));
        QVERIFY(clx + 21 < table.size() && table[clx] == 2);
        const int pieces = int((qFromLittleEndian<quint32>(table.constData() + clx + 1) - 4) / 12);
        QCOMPARE(pieces, 1);
        change(table, clx + 5, int(wd.size()));
        QVERIFY(f.setStream(QStringLiteral("1Table"), table));
        const QByteArray bad = jp::cfb::write(f);
        Document plain;
        QVERIFY(!read(bad, &plain) || bodyTexts(plain) != paras);
        QCOMPARE(repairedParagraphs(bad, &notes), paras);   // (the text is found without the table, whole)
        QVERIFY(notes.join(QLatin1Char(' ')).contains(QLatin1String("plain paragraphs")));
    }

private Q_SLOTS:
    void aTableWhoseTextEndsBeforeTheHeaderSaysIsNotTaken()
    {
        // the last position of the table says the text is 5 characters long, where the header says there are thousands
        tableWrongIn([](QByteArray &table, int plc, int) { qToLittleEndian(quint32(5), table.data() + plc + 4); });
    }

    void aTableWhoseFirstPieceStartsPastTheDocumentIsNotTaken()
    {
        // the piece's text is said to start far past the end of the document stream
        tableWrongIn([](QByteArray &table, int plc, int wdSize) { qToLittleEndian(quint32(wdSize + 100000), table.data() + plc + 8 + 2); });
    }

    void onlyAFewDirectoryEntriesOfOneNameAreTried()
    {
        jp::cfb::File f;
        jp::cfb::Entry root;
        root.type = jp::cfb::Entry::Root;
        root.name = QStringLiteral("Root Entry");
        f.entries.push_back(root);
        for (int i = 0; i < 300; ++i) {
            jp::cfb::Entry e;
            e.type = jp::cfb::Entry::Stream;
            e.name = QStringLiteral("X");
            e.data = QByteArray(5000 + i, 'a');
            f.entries[0].children.push_back(int(f.entries.size()));
            f.entries.push_back(e);
        }
        int calls = 0;
        jp::office::CfbWish w;
        w.name = QStringLiteral("X");
        w.validBytes = [&](QByteArrayView, const QMap<QString, QByteArray> &) -> qint64 {
            ++calls;
            return 0;
        };
        jp::office::CfbSalvage out;
        QString why;
        QVERIFY(!jp::office::salvageCompoundFile(jp::cfb::write(f), {w}, &out, &why));
        QVERIFY2(calls > 0 && calls <= 3 * jp::office::CfbSalvageLimits().maxEntryTries, qPrintable(QString::number(calls)));
    }

    void whatTheChecksAreGivenAddsUpToAFewTimesTheFile()
    {
        const QByteArray junk(200000, 'x');
        qint64 given = 0;
        int calls = 0;
        jp::office::CfbWish w;
        w.name = QStringLiteral("X");
        w.looksLike = [](QByteArrayView, const QMap<QString, QByteArray> &) { return true; };
        w.validBytes = [&](QByteArrayView b, const QMap<QString, QByteArray> &) -> qint64 {
            given += b.size();
            ++calls;
            return 0;
        };
        jp::office::CfbSalvageLimits lim;
        lim.maxChecks = 100000;
        jp::office::CfbSalvage out;
        QString why;
        QVERIFY(!jp::office::salvageCompoundFile(junk, {w}, &out, &why, lim));
        QVERIFY(calls > 1);
        QVERIFY2(given <= 8 * qint64(junk.size()) + (1 << 20), qPrintable(QString::number(given)));
    }

    void aMiniSectorOfTheWrongSizeMakesTheMiniStreamUnread()
    {
        jp::cfb::File f;
        jp::cfb::Entry root;
        root.type = jp::cfb::Entry::Root;
        root.name = QStringLiteral("Root Entry");
        f.entries.push_back(root);
        jp::cfb::Entry e;
        e.type = jp::cfb::Entry::Stream;
        e.name = QStringLiteral("S");
        e.data = QByteArray(300, 's');
        f.entries[0].children.push_back(1);
        f.entries.push_back(e);
        QByteArray file = jp::cfb::write(f);
        jp::office::CfbWish w;
        w.name = QStringLiteral("S");
        jp::office::CfbSalvage out;
        QString why;
        QVERIFY(jp::office::salvageCompoundFile(file, {w}, &out, &why));
        QVERIFY(!out.miniUnread);
        file[32] = 1;   // (2-byte mini sectors: the format has only 64)
        jp::office::salvageCompoundFile(file, {w}, &out, &why);
        QVERIFY(out.miniUnread);
        QVERIFY(!out.cutShort);
        const QString said = out.notes.join(QLatin1Char(' '));
        QVERIFY2(!said.contains(QLatin1String("cut short")) && said.contains(QLatin1String("mini stream")), qPrintable(said));
    }

    void aFileEndingInsideTheLastSectorOfAStreamIsCutShort()
    {
        jp::cfb::File f;
        jp::cfb::Entry root;
        root.type = jp::cfb::Entry::Root;
        root.name = QStringLiteral("Root Entry");
        f.entries.push_back(root);
        jp::cfb::Entry e;
        e.type = jp::cfb::Entry::Stream;
        e.name = QStringLiteral("S");
        e.data = QByteArray(10000, 's');
        f.entries[0].children.push_back(1);
        f.entries.push_back(e);
        const QByteArray whole = jp::cfb::write(f);
        const int entry = directoryEntryOf(whole, QStringLiteral("S"));
        QVERIFY(entry > 0);
        const int start = int(le32At(whole, entry + 116));
        jp::office::CfbWish w;
        w.name = QStringLiteral("S");
        jp::office::CfbSalvage out;
        QString why;
        // (the file ends 100 bytes into the stream's last sector, which is the 20th)
        QVERIFY(jp::office::salvageCompoundFile(whole.left((start + 20) * 512 + 100), {w}, &out, &why));
        QVERIFY(out.cutShort);
        QVERIFY(out.streams.value(QStringLiteral("S")).size() < 10000);
        QVERIFY(out.notes.join(QLatin1Char(' ')).contains(QLatin1String("cut short")));
    }

    // A developer's hook, not a test: JO_DOC_SWEEP_IN names a folder (searched through) of .doc files; each is read as it is, and again with bytes
    // of it changed, JO_DOC_SWEEP_ROUNDS times (20 if not said; the same changes each run unless JO_DOC_SWEEP_SEED says another), and what a sanitizer
    // build says is what it is for.
    void sweepOfRealFiles()
    {
        const QString in = qEnvironmentVariable("JO_DOC_SWEEP_IN");
        if (in.isEmpty()) QSKIP("JO_DOC_SWEEP_IN is not set");
        const int rounds = qEnvironmentVariableIsSet("JO_DOC_SWEEP_ROUNDS") ? qEnvironmentVariableIntValue("JO_DOC_SWEEP_ROUNDS") : 20;
        QRandomGenerator rng(qEnvironmentVariableIsSet("JO_DOC_SWEEP_SEED") ? quint32(qEnvironmentVariableIntValue("JO_DOC_SWEEP_SEED")) : 20261018u);
        int files = 0, readable = 0;
        QDirIterator it(in, {QStringLiteral("*.doc")}, QDir::Files, QDirIterator::Subdirectories);
        while (it.hasNext()) {
            QFile f(it.next());
            if (!f.open(QIODevice::ReadOnly)) continue;
            const QByteArray bytes = f.readAll();
            ++files;
            Document d;
            if (read(bytes, &d)) ++readable;
                QVERIFY2(doc::readWorkForTest().outputBytes <= doc::kMaxOutputBytes + (1 << 20) && doc::readWorkForTest().shapeVisits <= doc::kMaxShapeVisits + doc::kMaxShapes, qPrintable(f.fileName()));
            jp::cfb::File cf;
            const bool whole = jp::cfb::read(bytes, &cf);
            for (int round = 0; round < rounds; ++round) {
                QByteArray bad;
                if (whole && round % 2 == 0) {
                    jp::cfb::File g = cf;
                    const QStringList names = {QStringLiteral("WordDocument"), QStringLiteral("1Table"), QStringLiteral("0Table"), QStringLiteral("Data")};
                    const QString name = names[int(rng.bounded(quint32(names.size())))];
                    QByteArray st = g.stream(name);
                    if (st.isEmpty()) continue;
                    const int changes = 1 + int(rng.bounded(10));
                    for (int i = 0; i < changes; ++i) {
                        const int at = int(rng.bounded(quint32(st.size())));
                        switch (rng.bounded(3u)) {
                        case 0: st[at] = char(rng.bounded(256u)); break;
                        case 1: st[at] = char(0xFF); break;
                        default: st[at] = char(0); break;
                        }
                    }
                    g.setStream(name, st);
                    bad = jp::cfb::write(g);
                } else {
                    bad = bytes;
                    const int changes = 1 + int(rng.bounded(10));
                    for (int i = 0; i < changes; ++i) bad[int(rng.bounded(quint32(bad.size())))] = char(rng.bounded(256u));
                    if (round % 7 == 3) bad.truncate(int(rng.bounded(quint32(bad.size()))));
                }
                Document e;
                read(bad, &e);
                QVERIFY2(doc::readWorkForTest().outputBytes <= doc::kMaxOutputBytes + (1 << 20) && doc::readWorkForTest().shapeVisits <= doc::kMaxShapeVisits + doc::kMaxShapes, qPrintable(f.fileName()));
            }
        }
        qInfo("sweep: %d files, %d read whole, %d changed copies each", files, readable, rounds);
    }

    void changedStreamsNeverCrashOrHang()
    {
        // The same, with the changes made inside the streams (the compound file stays whole, so the damage reaches the reader's structures).
        DocWriter w = simple(QStringLiteral("Hello \x13 PAGE \x14""1\x15 world\a\a\rcell\a\a\r\x01 end\r"));
        w.styles = {};
        ListLevelSpec dec;
        dec.text = QString(QChar(0)) + QLatin1Char('.');
        w.lists = {{dec}};
        w.footnotes = {QStringLiteral("\x02 note\r")};
        w.footnoteRefs = {10};
        const int pic = w.addPicture(tinyPng(), 1440, 720);
        w.chars = {{0, 5, sprm1(sprm::bold, 1)}, {28, 29, sprm1(sprm::special, 1) + sprm4(sprm::picture, quint32(pic))}};
        const QByteArray in = sprm1(sprm::inTable, 1) + sprm4(sprm::tableDepth, 1);
        w.paras = {{0, 24, 0, sprm2(sprm::listInstance, 1)}, {24, 26, 0, in + sprm1(sprm::endsRow, 1) + tableRow({0, 1000, 2000})}};
        jp::cfb::File f;
        QVERIFY(jp::cfb::read(w.build(), &f));
        QRandomGenerator rng(20261017);
        const QStringList names = {QStringLiteral("WordDocument"), QStringLiteral("1Table"), QStringLiteral("Data")};
        // The places a change is made: the header's numbers of lengths and places, the first and last bytes of the tables it names (their counts and their
        // last places), and any byte that holds something; never the padding of zeros.
        const QByteArray wdBytes = f.stream(QStringLiteral("WordDocument")), tableBytes = f.stream(QStringLiteral("1Table"));
        QHash<QString, QVector<int>> aims;
        aims[names[0]] = fibNumbers(wdBytes) + liveBytes(wdBytes);
        aims[names[1]] = tableEdges(wdBytes, tableBytes) + liveBytes(tableBytes);
        aims[names[2]] = liveBytes(f.stream(QStringLiteral("Data")));
        QVERIFY(aims[names[0]].size() > 100 && aims[names[1]].size() > 100 && aims[names[2]].size() > 20);
        for (int round = 0; round < 600; ++round) {
            jp::cfb::File g = f;
            const QString name = names[int(rng.bounded(quint32(names.size())))];
            QByteArray s = g.stream(name);
            if (s.isEmpty()) continue;
            const QVector<int> &aim = aims[name];
            const int changes = 1 + int(rng.bounded(6));
            for (int i = 0; i < changes; ++i) {
                // (The first half of the aim is the numbers that count and measure; those go to the largest value a byte has, or the smallest, as often as to any.)
                const int at = aim[int(rng.bounded(quint32(aim.size())))];
                switch (rng.bounded(4u)) {
                case 0: s[at] = char(0xFF); break;
                case 1: s[at] = char(0x7F); break;
                case 2: s[at] = char(0); break;
                default: s[at] = char(rng.bounded(256u)); break;
                }
            }
            QVERIFY(g.setStream(name, s));
            Document d;
            QElapsedTimer one;
            one.start();
            read(jp::cfb::write(g), &d);
            // (Counted: whatever the bytes say, the work stays inside the reader's own limits.)
            const doc::ReadWork work = doc::readWorkForTest();
            QVERIFY2(work.outputBytes <= doc::kMaxOutputBytes + (1 << 20) && work.outputElements <= doc::kMaxOutputElements + 100 && work.shapeVisits <= doc::kMaxShapeVisits + doc::kMaxShapes
                         && work.picturesRead <= doc::kMaxPictures && work.styleSteps <= 4096 * (doc::kMaxStyleDepth + 2),
                     qPrintable(QStringLiteral("round %1: %2 bytes, %3 elements, %4 shape visits, %5 pictures, %6 style steps").arg(round).arg(work.outputBytes).arg(work.outputElements).arg(work.shapeVisits).arg(work.picturesRead).arg(work.styleSteps)));
            QVERIFY2(one.elapsed() < 3000, qPrintable(QStringLiteral("round %1 (%2) took %3 ms").arg(round).arg(name).arg(one.elapsed())));
        }
    }
};
JP_TEST_CLASS(OfficeDocTests)

#include "tst_doc.moc"
