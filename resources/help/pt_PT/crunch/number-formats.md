# Formatos de número e o seu idioma
<!-- keywords: formato de número, números, decimal, marca decimal, casas decimais, milhares, separador, agrupamento, vírgula, ponto, idioma, configuração regional, moeda, percentagem, data, hora, científico, fração, texto, contabilidade, geral, personalizado, código, exemplo, Formatar Células, Ctrl+1, Ctrl+Shift+4, Ctrl+Shift+5, aumentar casas decimais, diminuir casas decimais, estilo de vírgula, separador de fórmulas, ponto e vírgula -->

Uma célula contém um número, e o seu **formato de número** indica a forma como esse número é escrito no ecrã. O formato altera o que vê, nunca o que a célula contém. Exemplo: a célula contém 0,256. Com o formato Percentagem mostra **25,6%**, com Moeda mostra **0,26 $** e com Geral mostra **0,256**. Se clicar na célula, a barra de fórmulas continua a mostrar 0,256.

## Os números são escritos da forma como o seu computador os escreve

O JeffCrunch usa os sinais do idioma definido no seu computador. Em inglês (Estados Unidos), mil duzentos e trinta e quatro e meio escreve-se **1,234.5**. Em alemão escreve-se **1.234,5**, e em francês **1 234,5**. O livro não muda: mantém uma forma neutra, pelo que o mesmo ficheiro mostra 1,234.5 a uma pessoa e 1.234,5 a outra.

Isto aplica-se em todo o lado onde um número é mostrado ou escrito:

- Uma célula, a cópia de uma célula e a largura que **Ajustar Largura da Coluna** dá a uma coluna seguem os sinais do seu idioma.
- A **barra de fórmulas** e uma célula que edite com **F2** apresentam um número da forma como o escreve. Se o seu idioma escreve as casas decimais com vírgula, escreva **2,5**, e não 2.5.
- Num idioma que escreve as casas decimais com vírgula, as fórmulas usam **pontos e vírgulas** entre os argumentos: **=SOMA(A1;A2)**. Os nomes das funções mantêm-se em inglês.
- As datas e as horas seguem o código do formato: um formato que indica "mês, dia, ano" mostra-as nessa ordem, e um formato feito para o seu idioma (os predefinidos da lista) usa a ordem e os nomes do seu idioma.

O texto que o formato coloca entre aspas, como "unidades", é mostrado tal como está.

## Escolher um formato

1. Selecione as células.
2. Use o grupo **Número** no separador **Base**, ou prima **Ctrl+1** (no Mac, **Command+1**) para abrir **Formatar Células**, que abre no separador **Número**.

A caixa do grupo **Número** lista os formatos mais comuns: Geral, Número, Moeda, Contabilidade, Data Abreviada, Data por Extenso, Hora, Percentagem, Fração, Científico, Texto e Personalizado. **Mais Formatos Numéricos**, no fim da lista, abre a janela Formatar Células. Os botões abaixo da caixa são:

| Botão | O que faz |
| --- | --- |
| Moeda | Mostra o número como dinheiro, com o símbolo monetário do seu idioma (**Ctrl+Shift+4**). A seta ao lado oferece **Formato de Número de Contabilidade** com uma moeda à escolha (dólar americano, libra esterlina, euro, iene japonês, yuan chinês ou franco suíço), Geral e **Mais Formatos Numéricos**. |
| Estilo de Percentagem | Multiplica por 100 e acrescenta o sinal de percentagem, sem casas decimais (**Ctrl+Shift+5**). Clique novamente para o retirar. |
| Estilo de Vírgula | Agrupa os algarismos por milhares, mostra duas casas decimais e alinha as vírgulas decimais numa coluna, sem símbolo. |
| Aumentar Casas Decimais, Diminuir Casas Decimais | Acrescenta ou retira uma casa decimal. |
| Geral (na caixa, e na seta ao lado de Moeda) | Retira o formato: o número é mostrado tal como está (**Ctrl+Shift+~**). |

## O separador Número de Formatar Células

Escolha uma **Categoria** à esquerda; as opções correspondentes aparecem à direita, e o **Exemplo** mostra o aspeto que a primeira célula selecionada terá, no seu idioma.

- **Número**: casas decimais, se os algarismos são agrupados por milhares, e o aspeto dos números negativos (com sinal de menos, a vermelho ou entre parênteses).
- **Moeda** e **Contabilidade**: casas decimais e um símbolo. A Contabilidade alinha os símbolos e as vírgulas decimais numa coluna.
- **Data** e **Hora**: uma lista de formatos predefinidos, com a ordem e os nomes do seu idioma.
- **Percentagem** e **Científico**: casas decimais.
- **Fração**: formatos predefinidos como "até um algarismo" (1/4), "até dois algarismos" (21/25), "em quartos" e "em dezasseis avos".
- **Especial**: formatos predefinidos para um código postal, um código postal mais 4, um número de telefone e um número de segurança social.
- **Texto**: a célula contém o que escreve como texto, pelo que um número como 00123 mantém os zeros.
- **Personalizado**: escreva um código próprio, ou parta de um da lista (os códigos mais comuns e os que o livro já utiliza).

O **código** é o texto curto que indica ao JeffCrunch como escrever um número. Exemplo: **#,##0.00** significa "agrupar os algarismos por milhares e mostrar sempre duas casas decimais", pelo que 1234.5 aparece como 1,234.50. Cada **0** é um algarismo que aparece sempre, cada **#** é um algarismo que só aparece se for necessário, e **%** multiplica por 100. Um código pode ter até quatro secções separadas por ponto e vírgula, para números positivos, números negativos, zero e texto.

## O que é guardado

O código é guardado no livro na sua forma neutra, para que o livro se leia corretamente em qualquer computador. A formatação é um passo que pode anular, e um livro que já utilizava um formato mantém-no exatamente: um formato que está na lista é encontrado novamente, e nenhum é criado duas vezes.
