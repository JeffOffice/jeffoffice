# Formatos de número e o seu idioma
<!-- keywords: formato de número, formatar números, números, decimal, separador decimal, vírgula decimal, casas decimais, milhar, milhares, separador de milhar, agrupamento, vírgula, ponto, idioma, localidade, configurações regionais, moeda, real, porcentagem, percentual, data, hora, científico, fração, texto, contábil, geral, personalizado, código, exemplo, Formatar Células, Ctrl+1, Ctrl+Shift+4, Ctrl+Shift+5, aumentar casas decimais, diminuir casas decimais, separador de milhares, separador de fórmula, ponto e vírgula -->

Uma célula contém um número, e o **formato de número** dela determina como esse número é escrito na tela. O formato altera o que você vê, nunca o que a célula contém. Exemplo: a célula contém 0,256. Com o formato Porcentagem, ela mostra **25,6%**; com Moeda, mostra **R$ 0,26**; e com Geral, mostra **0,256**. Clique na célula e a barra de fórmulas continuará mostrando 0,256.

## Os números são escritos como o seu computador os escreve

O JeffCrunch usa os sinais do idioma configurado no seu computador. Em inglês (Estados Unidos), mil duzentos e trinta e quatro e meio é **1,234.5**. Em alemão é **1.234,5**, e em francês é **1 234,5**. A pasta de trabalho não muda: ela guarda uma forma neutra, de modo que o mesmo arquivo mostra 1,234.5 para uma pessoa e 1.234,5 para outra.

Isso vale em todos os lugares em que um número é mostrado ou digitado:

- Uma célula, a cópia de uma célula e a largura que **Ajustar largura da coluna** dá a uma coluna seguem os sinais do seu idioma.
- A **barra de fórmulas** e uma célula editada com **F2** mostram o número da forma como você o digita. Se o seu idioma escreve decimais com vírgula, digite **2,5**, e não 2.5.
- Em um idioma que escreve decimais com vírgula, uma fórmula tem **ponto e vírgula** entre os argumentos: **=SUM(A1;A2)**. Os nomes das funções permanecem em inglês.
- Datas e horas seguem o código do formato: um formato que diz "mês, dia, ano" as mostra nessa ordem, e um formato feito para o seu idioma (os prontos da lista) usa a ordem e os nomes do seu idioma.

O texto que o formato coloca entre aspas, como "unidades", é mostrado como está.

## Escolher um formato

1. Selecione as células.
2. Use o grupo **Número** da guia **Página Inicial** ou pressione **Ctrl+1** (no Mac, **Command+1**) para abrir **Formatar Células**, que abre na guia **Número**.

A caixa do grupo **Número** lista os formatos comuns: Geral, Número, Moeda, Contábil, Data Abreviada, Data Completa, Hora, Porcentagem, Fração, Científico, Texto e Personalizado. **Mais Formatos de Número**, no final, abre a janela Formatar Células. Os botões abaixo da caixa são:

| Botão | O que faz |
| --- | --- |
| Moeda | Mostra o número como dinheiro, com o símbolo de moeda do seu idioma (**Ctrl+Shift+4**). A seta ao lado oferece Geral e **Mais Formatos de Número**. |
| Estilo de Porcentagem | Multiplica por 100 e adiciona o sinal de porcentagem, sem casas decimais (**Ctrl+Shift+5**). Clique novamente para remover. |
| Separador de Milhares | Agrupa os dígitos por milhar, mostra duas casas decimais e alinha os separadores decimais ao longo da coluna, sem símbolo. |
| Aumentar Casas Decimais, Diminuir Casas Decimais | Adiciona ou remove uma casa decimal. |
| Geral (na caixa e na seta ao lado de Moeda) | Remove o formato: o número é mostrado como está (**Ctrl+Shift+~**). |

## A guia Número de Formatar Células

Escolha uma **Categoria** à esquerda; as opções dela aparecem à direita, e o **Exemplo** mostra como ficará a primeira célula selecionada, no seu idioma.

- **Número**: casas decimais, se os dígitos serão agrupados por milhar e como os números negativos aparecem (com sinal de menos, em vermelho ou entre parênteses).
- **Moeda** e **Contábil**: casas decimais e um símbolo. O formato Contábil alinha os símbolos e os separadores decimais ao longo da coluna.
- **Data** e **Hora**: uma lista de formatos prontos, com a ordem e os nomes do seu idioma.
- **Porcentagem**, **Científico**: casas decimais.
- **Fração**: formatos prontos como "até um dígito" (1/4), "até dois dígitos" (21/25), "em quartos" e "em dezesseis avos".
- **Especial**: formatos prontos para CEP, CEP + 4, número de telefone e número do seguro social.
- **Texto**: a célula guarda o que você digita como texto, de modo que um número como 00123 mantém os zeros.
- **Personalizado**: digite um código próprio ou comece por um da lista (os códigos comuns e os que a pasta de trabalho já usa).

O **código** é o texto curto que diz ao JeffCrunch como escrever um número. Exemplo: **#,##0.00** significa "agrupar os dígitos por milhar e sempre mostrar duas casas decimais", de modo que 1234,5 aparece como 1.234,50 (em um idioma que escreve decimais com vírgula). Cada **0** é um dígito sempre mostrado, cada **#** é um dígito mostrado somente se necessário, e **%** multiplica por 100. Um código pode ter até quatro seções separadas por ponto e vírgula: para números positivos, números negativos, zero e texto.

## O que é mantido

O código é salvo na pasta de trabalho na forma neutra, para que a pasta de trabalho seja lida corretamente em qualquer computador. A formatação é uma etapa que você pode desfazer, e uma pasta de trabalho que já usava um formato o mantém exatamente: um formato que está na lista é encontrado novamente, e nenhum é criado em duplicidade.
