# Localizar e substituir palavras
<!-- keywords: localizar, substituir, substituir tudo, pesquisar, procurar, buscar, alterar palavras, trocar palavras, diferenciar maiúsculas de minúsculas, palavras inteiras, localizar próxima, ctrl+f, ctrl+h, edição, painel, texto oculto, desfazer -->

Use Localizar para ir até palavras no seu documento. Use Substituir para alterá-las. Os dois funcionam em um painel à direita da janela.

## Localizar palavras

1. Pressione **Ctrl+F** ou clique em **Página Inicial > Edição > Localizar**. O painel **Localizar e Substituir** é aberto, com o cursor em **Localizar**.
2. Digite as palavras que você procura. Em seguida, pressione **Enter** ou clique em **Localizar Próxima**.

O JeffWrite começa onde está o cursor. Ele seleciona o próximo local em que encontra as palavras e rola as páginas para que você possa vê-lo. Esse local é chamado de **ocorrência**. Clique em **Localizar Próxima** novamente para ir até a próxima ocorrência. Depois da última ocorrência, ele volta ao início do documento.

Se as palavras não estiverem no documento, o painel informa isso e nada se move.

Você pode continuar trabalhando no documento com o painel aberto. Clique na página para digitar. Clique no **X** na parte superior do painel para fechá-lo.

## Refinar a pesquisa

- **Diferenciar maiúsculas de minúsculas** localiza somente palavras com as mesmas letras maiúsculas. Com essa opção ativada, "Plano" não localiza "plano".
- **Localizar apenas palavras inteiras** ignora palavras mais longas que contêm as suas. Com essa opção ativada, "plano" não localiza "planos".

## Substituir palavras

1. Pressione **Ctrl+H** ou clique em **Página Inicial > Edição > Substituir**. O cursor vai para **Substituir por**.
2. Digite as novas palavras em **Substituir por**. Para remover as palavras encontradas, deixe a caixa vazia.
3. Digite as palavras antigas em **Localizar**.
4. Clique em **Substituir** para ver a primeira ocorrência. Clique em **Substituir** novamente para alterá-la. Em seguida, o JeffWrite vai para a próxima ocorrência. Cada alteração é uma etapa, então **Desfazer** (Ctrl+Z) desfaz uma de cada vez.
5. Clique em **Substituir Tudo** para alterar todas as ocorrências de uma só vez. O painel informa quantas foram alteradas. **Substituir Tudo** é uma única etapa, então um único **Desfazer** restaura todas as palavras.

As novas palavras ficam com a aparência da primeira letra das palavras que substituem. Se você substituir uma palavra em negrito, a nova palavra ficará em negrito.

Por exemplo, para trocar "cor" por "tom" em todo o documento: digite **cor** em **Localizar**, digite **tom** em **Substituir por** e clique em **Substituir Tudo**.

## Onde a pesquisa procura

- Ela procura no texto do documento, incluindo o texto das tabelas. Um indicador no meio de algumas palavras, presente em alguns arquivos, não impede que elas sejam encontradas. **Substituir** e **Substituir Tudo** mantêm o indicador.
- Ela ainda não procura em cabeçalhos, rodapés, notas de rodapé ou notas de fim. As palavras nesses locais não são encontradas, e **Substituir Tudo** não as altera. Se um cabeçalho ou rodapé estiver aberto para edição quando você abrir o painel, ele será fechado primeiro. Consulte [Cabeçalhos e rodapés](headers-footers).
- Ela ignora o texto definido como **Oculto**, porque esse texto não é exibido. Consulte [Alterar a aparência do texto](font).

## Tópicos relacionados

- [Alterar a aparência do texto](font)
- [Usar o JeffWrite com o teclado](keyboard)
