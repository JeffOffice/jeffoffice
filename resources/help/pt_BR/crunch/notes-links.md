# Notas, comentários e links

<!-- keywords: painel de comentários, novo comentário, responder, resolver, reabrir, conversa, comentário anterior, próximo comentário, mostrar comentários, converter anotações em comentários, converter comentários em anotações, dicionário de sinônimos, sinônimos, lista de palavras, nota, notas, comentário, comentários, passar o ponteiro, marca vermelha, canto, nova nota, editar nota, excluir nota, mostrar nota, ocultar nota, mostrar todas as notas, link, links, hiperlink, endereço, página da web, email, e-mail, local nesta pasta de trabalho, Ctrl+K, Ctrl+clique, dica de tela, abrir link, remover link, discussão, estatísticas da pasta de trabalho, contar, células, fórmulas, gráficos, tabelas dinâmicas, acessibilidade, texto alternativo, células mescladas, arquivos recentes -->

## Notas

Uma **nota** é um pequeno texto guardado junto de uma célula. Uma célula que tem uma nota mostra uma pequena marca vermelha no canto superior direito. Passe o ponteiro sobre a célula, e a nota aparece ao lado dela; afaste o ponteiro, e ela some.

Para colocar uma nota em uma célula, selecione-a e use **Revisão > Nova Anotação** (ou pressione **Shift+F2**). Digite o texto e clique em **OK**. Em uma célula que já tem nota, o mesmo comando aparece como **Editar Anotação**. Apagar o texto remove a nota.

- **Excluir Anotação** remove as notas das células selecionadas.
- **Mostrar ou Ocultar Anotação** mantém a nota das células selecionadas visível na planilha o tempo todo, em uma caixa ao lado de cada célula, ou volta a escondê-la.
- **Mostrar Todas as Anotações** faz o mesmo para todas as notas da planilha; o mesmo botão passa a se chamar **Ocultar Todas as Anotações**.
- **Anotação Anterior** e **Próxima Anotação** levam de uma célula com nota para a seguinte.

Cada uma dessas ações pode ser desfeita em um único passo. Uma nota acompanha a sua célula quando linhas ou colunas são inseridas ou excluídas e quando as células são classificadas. **Limpar Tudo** remove as notas e os comentários das células que limpa.

Uma célula também pode ter uma conversa de **comentários**, com respostas. Veja a próxima parte.

As notas podem ser salvas em uma pasta de trabalho .xlsx. Uma pasta de trabalho aberta a partir de um tipo de arquivo mais antigo, ou de um arquivo de texto, mantém as notas somente depois que você a salvar como pasta de trabalho .xlsx.

## Comentários

Um **comentário** é parecido com uma nota, mas inicia uma **discussão**: as pessoas podem respondê-lo, e uma delas pode marcar a discussão como **resolvida** quando a pergunta for respondida. Cada comentário mostra quem o escreveu e quando. Uma célula com discussão tem a mesma marca vermelha de uma célula com nota. Passe o ponteiro sobre a célula para ler a discussão inteira, com cada resposta recuada abaixo do comentário que ela responde. Uma discussão resolvida começa com a palavra **Resolvido**.

Na guia **Revisão**, no grupo **Comentários**:

- **Novo Comentário** (**Ctrl+Shift+F2**) inicia uma discussão na célula selecionada. Digite o texto e clique em **OK**. Funciona em uma célula que não tem nota nem comentários.
- **Responder** acrescenta o seu texto à discussão da célula selecionada.
- **Resolver Discussão** marca a discussão como concluída. O mesmo botão passa a se chamar **Reabrir Discussão**, que a abre novamente.
- **Excluir Comentário** remove as discussões das células selecionadas.
- **Comentário Anterior** e **Próximo Comentário** levam de uma célula com discussão para a seguinte.
- **Mostrar Comentários** abre o painel **Comentários** ao lado da planilha. Ele lista cada discussão da planilha como um cartão: a célula, cada comentário com seu autor e hora, e os botões **Responder**, **Resolver** ou **Reabrir**, e **Excluir**. **Ir Para** leva até a célula. O painel acompanha cada alteração, inclusive quando você usa Desfazer.
- **Converter Anotações em Comentários** transforma cada nota simples da planilha em uma discussão de um comentário. **Converter Comentários em Anotações** transforma cada discussão de volta em uma nota. Uma nota não tem respostas, por isso os comentários de uma discussão viram uma nota com todo o texto deles, e o JeffCrunch pede confirmação antes de fazer isso com uma discussão que tem respostas.

Cada uma dessas ações pode ser desfeita em um único passo.

Programas que não conseguem mostrar discussões exibem uma nota simples no lugar delas. O JeffCrunch mantém essa nota, com o texto da discussão, a cada vez que salva. Discussões, respostas e as pessoas que as escreveram permanecem como estavam quando você salva e abre a pasta de trabalho novamente. Uma discussão com mais de 500 comentários é exibida e mantida, mas o JeffCrunch não acrescenta nada a ela nem a altera.

Os comentários são salvos em uma pasta de trabalho .xlsx. Uma pasta de trabalho aberta a partir de um tipo de arquivo mais antigo, ou de um arquivo de texto, mantém os comentários somente depois que você a salvar como pasta de trabalho .xlsx.

## Links

Um **link** leva você a algum lugar quando você o segue: a uma página da web, a um arquivo, a um endereço de email ou a um local nesta pasta de trabalho (uma célula, células ou um nome).

Para colocar um link em uma célula, selecione-a e use **Inserir > Link** (ou pressione **Ctrl+K**). Escolha para onde ele leva, digite o endereço ou o local (ou escolha um dos arquivos abertos recentemente em **Ou um arquivo recente**) e, se quiser, o texto que a célula deve mostrar e uma **dica de tela**. Em uma célula que já tem link, o mesmo comando aparece como **Editar Link**. **Remover Link** tira os links das células selecionadas.

Passe o ponteiro sobre uma célula com link para ver para onde ele leva. Para segui-lo, mantenha **Ctrl** pressionada e clique na célula, ou selecione a célula e use **Abrir Link** no menu do botão direito. Um clique simples apenas seleciona a célula.

O JeffCrunch nunca abre nada por conta própria. Uma página da web ou um endereço de email abre no seu programa habitual quando você segue o link. Um link para um arquivo, ou para outro tipo de endereço, pede a sua confirmação antes. Um link que executaria um script nunca é aberto.

Um link acompanha a sua célula quando linhas ou colunas são inseridas ou excluídas e quando as células são classificadas, e é salvo junto com a pasta de trabalho.

## Estatísticas da Pasta de Trabalho

**Estatísticas da Pasta de Trabalho** (guia **Revisão**) conta o que a pasta de trabalho contém: quantas **Planilhas** ela tem, quantas **Células com dados** (células que contêm um número, um texto ou uma fórmula), quantas **Fórmulas**, quantos **Gráficos** e quantas **Tabelas dinâmicas**. As planilhas excluídas não são contadas. Os mesmos números aparecem na página **Informações** da guia Arquivo.

## Dicionário de Sinônimos

**Dicionário de Sinônimos** (guia **Revisão**, ou **Shift+F7**) encontra palavras de sentido quase igual ao de uma palavra da célula. Ele procura a primeira palavra do texto da célula em que você está. Digite outra palavra em **Pesquisar** e pressione **Enter** para pesquisar essa palavra. Escolha uma palavra na lista e clique em **Substituir**, ou clique duas vezes nela. A palavra da célula é substituída, e uma primeira letra maiúscula continua maiúscula. **Pesquisar** procura a palavra escolhida, para seguir uma cadeia de significados. A lista de palavras funciona em inglês.

## Verificar Acessibilidade

**Verificar Acessibilidade** (guia **Revisão**) abre uma pequena janela que lista o que na pasta de trabalho é difícil de usar para alguns leitores, ou para um leitor de tela (um programa que lê a tela em voz alta). Cada linha começa com **Erro**, **Aviso** ou **Dica**. Clique em uma linha para ir até a célula.

Ele procura:

- Um gráfico ou imagem sem **texto alternativo** (as palavras que dizem o que ele mostra). Isso é um erro.
- Uma tabela sem linha de cabeçalho, de modo que suas colunas não têm nomes. Isso é um erro.
- **Células mescladas**, que são difíceis de percorrer com o teclado. Isso é um aviso.
- Texto difícil de ler sobre a cor de fundo, como texto cinza-claro sobre branco. O JeffCrunch mede o quanto o texto se destaca do preenchimento. Textos pequenos precisam de uma razão de pelo menos 4,5 para 1, e textos grandes (18 pontos, ou negrito de 14 pontos) precisam de 3 para 1. Ele indica a primeira célula de cada combinação de fonte e preenchimento. Isso é um aviso.
- Uma planilha que ainda tem o nome que recebeu, como Planilha1. Isso é uma dica.

Clique em **Verificar Novamente** depois de fazer alterações.
