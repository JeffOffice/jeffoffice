# Formatos de número e o seu idioma
<!-- keywords: formato de número, números, decimal, separador decimal, casas decimais, milhares, separador de milhares, agrupamento, vírgula, ponto, idioma, configuração regional, moeda, percentual, porcentagem, data, hora, científico, fração, texto, contábil, geral, personalizado, código, exemplo, Formatar Células, Ctrl+1, Ctrl+Shift+4, Ctrl+Shift+5, aumentar casas decimais, diminuir casas decimais, estilo vírgula, separador de fórmula, ponto e vírgula -->

Uma célula contém um número, e o **formato de número** dele define como esse número é escrito na tela. O formato muda o que você vê, nunca o que a célula contém. Exemplo: a célula contém 0,256. Com o formato Porcentagem, ela mostra **25,6%**; com Moeda, mostra **R$ 0,26**; e com Geral, mostra **0,256**. Clique na célula e a barra de fórmulas continua mostrando 0,256.

## Os números são escritos do jeito que o seu computador os escreve

O JeffCrunch usa os separadores do idioma configurado no seu computador. Em português (Brasil), mil duzentos e trinta e quatro vírgula cinco é escrito **1.234,5**. Em inglês (Estados Unidos), é **1,234.5**, e em francês, **1 234,5**. A pasta de trabalho não muda: ela guarda uma forma neutra, de modo que o mesmo arquivo mostra 1.234,5 para uma pessoa e 1,234.5 para outra.

Isso vale em todo lugar onde um número é mostrado ou digitado:

- Uma célula, a cópia de uma célula e a largura que **Ajustar Largura da Coluna** dá a uma coluna seguem os separadores do seu idioma.
- A **barra de fórmulas** e uma célula editada com **F2** mostram o número do jeito que você o digita. Se o seu idioma usa vírgula para a casa decimal, digite **2,5**, não 2.5.
- Em um idioma que escreve decimais com vírgula, uma fórmula tem **ponto e vírgula** entre os argumentos: **=SUM(A1;A2)**. Os nomes das funções permanecem em inglês.
- Datas e horas seguem o código do formato: um formato que diz "mês, dia, ano" as mostra nessa ordem, e um formato feito para o seu idioma (os prontos da lista) usa a ordem e os nomes do seu idioma.

Texto que o formato coloca entre aspas, como "unidades", é mostrado como está.

## Escolha um formato

1. Selecione as células.
2. Use o grupo **Número** na guia **Página Inicial**, ou pressione **Ctrl+1** (no Mac, **Command+1**) para abrir **Formatar Células**, que abre na guia **Número**.

A caixa do grupo **Número** lista os formatos comuns: Geral, Número, Moeda, Contábil, Data Abreviada, Data Completa, Hora, Porcentagem, Fração, Científico, Texto e Personalizado. **Mais Formatos de Número** no fim abre a janela Formatar Células. Os botões abaixo da caixa são:

| Botão | O que faz |
| --- | --- |
| Moeda | Mostra o número como dinheiro, com o símbolo monetário do seu idioma (**Ctrl+Shift+4**). A seta ao lado oferece **Formato de Número Contábil** com uma moeda para escolher (dólar americano, libra esterlina, euro, iene japonês, yuan chinês ou franco suíço), Geral e **Mais Formatos de Número**. |
| Estilo de Porcentagem | Multiplica por 100 e acrescenta o sinal de porcentagem, sem casas decimais (**Ctrl+Shift+5**). Clique de novo para removê-lo. |
| Estilo Vírgula | Agrupa os dígitos por milhares, mostra duas casas decimais e alinha os separadores decimais em uma coluna, sem símbolo. |
| Aumentar Casas Decimais, Diminuir Casas Decimais | Acrescenta ou tira uma casa decimal. |
| Geral (na caixa, e na seta ao lado de Moeda) | Remove o formato: o número aparece como é (**Ctrl+Shift+~**). |

## A guia Número de Formatar Células

Escolha uma **Categoria** à esquerda; as opções dela aparecem à direita, e a **Amostra** mostra como ficará a primeira célula selecionada, no seu idioma.

- **Número**: casas decimais, se os dígitos são agrupados por milhares, e como os números negativos aparecem (com sinal de menos, em vermelho ou entre parênteses).
- **Moeda** e **Contábil**: casas decimais e um símbolo. O Contábil alinha os símbolos e os separadores decimais em uma coluna.
- **Data** e **Hora**: uma lista de formatos prontos, com a ordem e os nomes do seu idioma.
- **Porcentagem**, **Científico**: casas decimais.
- **Fração**: formatos prontos como "até um dígito" (1/4), "até dois dígitos" (21/25), "em quartos" e "em dezesseis avos".
- **Especial**: formatos prontos para CEP, CEP + 4, número de telefone e número do seguro social.
- **Texto**: a célula guarda o que você digita como texto, então um número como 00123 mantém os zeros.
- **Personalizado**: digite um código seu, ou comece de um da lista (os códigos comuns e os que a pasta de trabalho já usa).

O **código** é o texto curto que diz ao JeffCrunch como escrever um número. Exemplo: **#.##0,00** significa "agrupar os dígitos por milhares e sempre mostrar duas casas decimais", então 1234,5 aparece como 1.234,50. Cada **0** é um dígito que sempre aparece, cada **#** é um dígito que aparece só se for necessário, e **%** multiplica por 100. Um código pode ter até quatro seções separadas por ponto e vírgula, para números positivos, números negativos, zero e texto.

## O que é mantido

O código é salvo na pasta de trabalho na forma neutra, para que ela seja lida corretamente em qualquer computador. A formatação é uma etapa que você pode desfazer, e uma pasta de trabalho que já usava um formato o mantém exatamente: um formato que está na lista é encontrado de novo, e nenhum é criado duas vezes.
