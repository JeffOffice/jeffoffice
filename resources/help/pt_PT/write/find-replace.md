# Localizar e substituir palavras
<!-- keywords: localizar, substituir, substituir tudo, procurar, pesquisar, alterar palavras, maiúsculas e minúsculas, palavras inteiras, localizar seguinte, ctrl+f, ctrl+h, edição, painel, texto oculto, anular -->

Utilize Localizar para ir para palavras do documento. Utilize Substituir para as alterar. Ambos funcionam num painel à direita da janela.

## Localizar palavras

1. Prima **Ctrl+F** ou clique em **Base > Edição > Localizar**. O painel **Localizar e Substituir** abre, com o cursor em **Localizar**.
2. Escreva as palavras pretendidas. Em seguida, prima **Enter** ou clique em **Localizar Seguinte**.

O JeffWrite começa onde está o cursor. Seleciona o local seguinte onde encontra as palavras e desloca as páginas para que o possa ver. Esse local chama-se **correspondência**. Clique novamente em **Localizar Seguinte** para ir para a correspondência seguinte. Depois da última correspondência, volta ao início do documento.

Se as palavras não estiverem no documento, o painel indica-o e nada se move.

Pode continuar a trabalhar no documento enquanto o painel está aberto. Clique na página para escrever. Clique no **X** na parte superior do painel para o fechar.

## Restringir a pesquisa

- **Maiúsculas/minúsculas** localiza apenas palavras com as mesmas letras maiúsculas. Com esta opção ativada, "Plano" não localiza "plano".
- **Localizar apenas palavras inteiras** ignora palavras mais longas que contêm as suas. Com esta opção ativada, "mar" não localiza "marca".

## Substituir palavras

1. Prima **Ctrl+H** ou clique em **Base > Edição > Substituir**. O cursor passa para **Substituir por**.
2. Escreva as novas palavras em **Substituir por**. Para remover as palavras encontradas, deixe a caixa vazia.
3. Escreva as palavras antigas em **Localizar**.
4. Clique em **Substituir** para ver a primeira correspondência. Clique novamente em **Substituir** para a alterar. Em seguida, o JeffWrite passa para a correspondência seguinte. Cada alteração é um passo, pelo que **Anular** (Ctrl+Z) desfaz uma de cada vez.
5. Clique em **Substituir Tudo** para alterar todas as correspondências de uma só vez. O painel indica quantas foram alteradas. **Substituir Tudo** é um único passo, pelo que um único **Anular** repõe todas as palavras.

As novas palavras ficam com o aspeto da primeira letra das palavras que substituem. Se substituir uma palavra em negrito, a nova palavra fica em negrito.

Por exemplo, para alterar "fato" para "facto" em todo o documento: escreva **fato** em **Localizar**, escreva **facto** em **Substituir por** e clique em **Substituir Tudo**.

## O que a pesquisa analisa

- Analisa o texto do documento, incluindo o texto das tabelas. Um marcador no meio de algumas palavras, que alguns ficheiros têm, não impede que estas sejam encontradas. **Substituir** e **Substituir Tudo** mantêm o marcador.
- Ainda não analisa cabeçalhos, rodapés, notas de rodapé nem notas de fim. As palavras aí não são encontradas e **Substituir Tudo** não as altera. Se um cabeçalho ou rodapé estiver aberto para edição quando abrir o painel, este fecha primeiro. Consulte [Cabeçalhos e rodapés](headers-footers).
- Ignora o texto definido como **Oculto**, porque esse texto não é mostrado. Consulte [Alterar o aspeto do texto](font).

## Relacionados

- [Alterar o aspeto do texto](font)
- [Utilizar o JeffWrite com o teclado](keyboard)
