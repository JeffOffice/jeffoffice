# Formatos de número e o seu idioma
<!-- keywords: formato de número, formatos numéricos, números, decimal, separador decimal, vírgula decimal, casas decimais, milhares, separador de milhares, agrupamento, vírgula, ponto, idioma, região, definições regionais, moeda, euro, percentagem, data, hora, científico, fração, texto, contabilidade, geral, personalizado, código, exemplo, Formatar Células, Ctrl+1, Ctrl+Shift+4, Ctrl+Shift+5, aumentar casas decimais, diminuir casas decimais, estilo de vírgula, separador de fórmulas, ponto e vírgula -->

Uma célula contém um número e o respetivo **formato de número** indica como esse número é escrito no ecrã. O formato altera o que vê, nunca o que a célula contém. Exemplo: a célula contém 0,256. Com o formato Percentagem mostra **25,6%**, com Moeda mostra **0,26 €** e com Geral mostra **0,256**. Clique na célula e a barra de fórmulas continua a mostrar 0,256.

## Os números são escritos tal como o seu computador os escreve

O JeffCrunch utiliza os sinais do idioma definido no seu computador. Em inglês (Estados Unidos), mil duzentos e trinta e quatro e meio escreve-se **1,234.5**. Em alemão é **1.234,5** e em francês é **1 234,5**. O livro não muda: guarda uma forma neutra, pelo que o mesmo ficheiro mostra 1,234.5 a uma pessoa e 1.234,5 a outra.

Isto aplica-se a todos os locais onde um número é mostrado ou escrito:

- Uma célula, a cópia de uma célula e a largura que o **Ajuste automático** dá a uma coluna seguem os sinais do seu idioma.
- A **barra de fórmulas** e uma célula que edita com **F2** aceitam um número tal como o escreve. Se o seu idioma escreve os decimais com vírgula, escreva **2,5** e não 2.5.
- Num idioma que escreve os decimais com vírgula, uma fórmula tem **pontos e vírgulas** entre os argumentos: **=SUM(A1;A2)**. Os nomes das funções mantêm-se em inglês.
- As datas e horas seguem o código do formato: um formato que indica "mês, dia, ano" mostra-as por essa ordem, e um formato próprio do seu idioma (os formatos prontos da lista) utiliza a ordem e os nomes do seu idioma.

O texto que o formato coloca entre aspas, como "unidades", é mostrado tal como está.

## Escolher um formato

1. Selecione as células.
2. Utilize o grupo **Número** do separador **Base** ou prima **Ctrl+1** (num Mac, **Command+1**) para abrir **Formatar Células**, que abre no separador **Número**.

A caixa do grupo **Número** lista os formatos comuns: Geral, Número, Moeda, Contabilidade, Data Abreviada, Data por Extenso, Hora, Percentagem, Fração, Científico, Texto e Personalizado. **Mais Formatos Numéricos…**, no fim da lista, abre a janela Formatar Células. Os botões por baixo da caixa são:

| Botão | O que faz |
| --- | --- |
| Moeda | Mostra o número como dinheiro, com o símbolo monetário do seu idioma (**Ctrl+Shift+4**). A seta ao lado oferece Geral e **Mais Formatos Numéricos…**. |
| Estilo de Percentagem | Multiplica por 100 e acrescenta um sinal de percentagem, sem casas decimais (**Ctrl+Shift+5**). Clique novamente para o remover. |
| Estilo de Vírgula | Agrupa os dígitos por milhares, mostra duas casas decimais e alinha os separadores decimais ao longo de uma coluna, sem símbolo. |
| Aumentar Casas Decimais, Diminuir Casas Decimais | Acrescenta ou retira uma casa decimal. |
| Geral (na caixa e na seta ao lado de Moeda) | Remove o formato: o número é mostrado tal como está (**Ctrl+Shift+~**). |

## O separador Número de Formatar Células

Escolha uma **Categoria** à esquerda; as opções correspondentes aparecem à direita e o **Exemplo** mostra o aspeto que a primeira célula selecionada terá, no seu idioma.

- **Número**: casas decimais, se os dígitos são agrupados por milhares e o aspeto dos números negativos (sinal de menos, a vermelho ou entre parênteses).
- **Moeda** e **Contabilidade**: casas decimais e um símbolo. Contabilidade alinha os símbolos e os separadores decimais ao longo de uma coluna.
- **Data** e **Hora**: uma lista de formatos prontos, com a ordem e os nomes do seu idioma.
- **Percentagem**, **Científico**: casas decimais.
- **Fração**: formatos prontos como "Até um dígito" (1/4), "Até dois dígitos" (21/25), "Como quartos" e "Como dezasseis avos".
- **Especial**: formatos prontos para um código postal, um código postal + 4, um número de telefone e um número de segurança social.
- **Texto**: a célula guarda o que escreve como texto, pelo que um número como 00123 mantém os zeros.
- **Personalizado**: escreva um código próprio ou comece a partir de um da lista (os códigos comuns e os que o livro já utiliza).

O **código** é o texto curto que indica ao JeffCrunch como escrever um número. Exemplo: **#,##0.00** significa "agrupar os dígitos por milhares e mostrar sempre duas casas decimais", pelo que 1234,5 é mostrado como 1.234,50 (ou 1 234,50, conforme o seu idioma). Cada **0** é um dígito que é sempre mostrado, cada **#** um dígito que só é mostrado se for necessário, e **%** multiplica por 100. Um código pode ter até quatro secções separadas por pontos e vírgulas, para números positivos, números negativos, zero e texto.

## O que é mantido

O código é guardado no livro na sua forma neutra, para que o livro seja lido corretamente em qualquer computador. A formatação é um passo que pode anular, e um livro que já utilizava um formato mantém-no exatamente: um formato que está na lista é reencontrado e nenhum é criado em duplicado.
