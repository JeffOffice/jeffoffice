# Referências cruzadas
<!-- keywords: referência cruzada, referências cruzadas, remissão, REF, PAGEREF, NOTEREF, ver página, consulte a página, ver figura, acima, abaixo, número do título, link, vínculo -->

Uma **referência cruzada** é um trecho do texto que aponta para outro local, como "consulte a Figura 2 na página 7". Ela é um campo, então, quando a figura ou a página muda, você atualiza o campo e o texto acompanha a mudança.

## Inserir uma referência cruzada

1. Clique no local onde o texto deve ficar.
2. Clique em **Referências > Referência cruzada**.
3. Escolha o **Tipo de referência**: um título, um indicador, uma nota de rodapé, uma nota de fim, um item numerado ou uma legenda, como Figura ou Tabela.
4. Escolha o que usar em **Inserir referência para**: o texto, o número, o número da página ou "acima" e "abaixo".
5. Escolha o item na lista, deixe **Inserir como hiperlink** marcado se quiser poder saltar até ele e clique em **Inserir**.

Por exemplo, para escrever "Consulte o título Resultados na página 4": digite `Consulte o título `, insira uma referência ao título com **Texto do título**, digite ` na página ` e insira uma segunda referência ao mesmo título com **Número de página**.

## Seguir uma referência

Mantenha a tecla **Ctrl** pressionada e clique em uma referência que seja um hiperlink. O cursor vai para o local indicado.

## Manter o texto correto

Depois de adicionar páginas, alterar títulos ou mover figuras, selecione o texto e pressione **F9**. Cada referência recalcula o seu texto. Se o local para o qual uma referência aponta tiver sido excluído, o campo mostrará `Erro: o indicador ao qual isto se refere está ausente.`, e você poderá escolher um novo destino.

## O que acontece nos bastidores

O JeffWrite marca o destino com um indicador oculto (cujo nome começa com `_Ref`) e cria um campo que se refere a ele: `REF` para texto e números, `PAGEREF` para números de página e `NOTEREF` para notas. Outros programas abrem o arquivo com as referências ainda funcionando.
