# Referências cruzadas
<!-- keywords: referência cruzada, referências cruzadas, remissão, REF, PAGEREF, NOTEREF, ver página, ver figura, acima, abaixo, número do título, ligação, hiperligação -->

Uma **referência cruzada** é um conjunto de palavras no texto que remete para outro local, como "ver Figura 2 na página 7". Trata-se de um campo, pelo que, quando a figura ou a página muda, basta atualizar o campo para que as palavras acompanhem a alteração.

## Inserir uma referência cruzada

1. Clique onde pretende colocar as palavras.
2. Clique em **Referências > Referência cruzada**.
3. Escolha o **Tipo de referência**: um título, um marcador, uma nota de rodapé, uma nota de fim, um item numerado ou uma legenda, como Figura ou Tabela.
4. Escolha o que pretende em **Inserir referência a**: o texto, o número, o número de página ou "acima" e "abaixo".
5. Escolha o item na lista, deixe **Inserir como hiperligação** selecionado se pretender saltar para o item e clique em **Inserir**.

Por exemplo, para escrever "Ver o título Resultados na página 4": escreva `Ver o título `, insira uma referência ao título com **Texto do título**, escreva ` na página ` e insira uma segunda referência ao mesmo título com **Número de página**.

## Seguir uma referência

Mantenha premida a tecla **Ctrl** e clique numa referência que seja uma hiperligação. O cursor vai para o local indicado.

## Manter as palavras corretas

Depois de adicionar páginas, alterar títulos ou mover figuras, selecione o texto e prima **F9**. Todas as referências voltam a calcular as respetivas palavras. Se o local para onde uma referência remete tiver sido eliminado, o campo mostra `Erro: o marcador a que isto se refere não existe.` e pode escolher um novo destino.

## O que acontece nos bastidores

O JeffWrite marca o destino com um marcador oculto (cujo nome começa por `_Ref`) e cria um campo que lhe faz referência: `REF` para texto e números, `PAGEREF` para números de página e `NOTEREF` para notas. Outros programas abrem o ficheiro com as referências a funcionar.
