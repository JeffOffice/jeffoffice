# Notas, comentários e links
<!-- keywords: mover nota, redimensionar nota, tamanho da nota, caixa de nota, redefinir tamanho da nota, teclado, idioma, idioma de revisão de texto, definir idioma, não verificar ortografia, traduzir, tradução, libretranslate, painel de comentários, novo comentário, responder, resolver, reabrir, discussão, comentário anterior, próximo comentário, mostrar comentários, converter notas em comentários, converter comentários em notas, ortografia, verificar ortografia, palavra errada, F7, ignorar tudo, alterar tudo, adicionar ao dicionário, dicionário de sinônimos, sinônimos, lista de palavras, nota, notas, comentário, comentários, passar o ponteiro, marca vermelha, canto, nova nota, editar nota, excluir nota, mostrar nota, ocultar nota, mostrar todas as notas, link, links, hiperlink, endereço, página da web, email, local nesta pasta de trabalho, Ctrl+K, Ctrl+clique, dica de tela, abrir link, remover link, conversa, estatísticas da pasta de trabalho, contar, células, fórmulas, gráficos, tabelas dinâmicas, acessibilidade, texto alternativo, células mescladas, arquivos recentes -->

## Notas

Uma **nota** é um pequeno texto guardado junto a uma célula. A célula que tem uma nota mostra uma pequena marca vermelha no canto superior direito. Ao apontar para a célula, a nota aparece ao lado dela; ao afastar o ponteiro, ela some.

Para colocar uma nota em uma célula, selecione-a e use **Revisão > Nova Nota** (ou pressione **Shift+F2**). Digite o texto e clique em **OK**. Em uma célula que já tem nota, o mesmo comando aparece como **Editar Nota**. Apagar o texto remove a nota.

- **Excluir Nota** remove as notas das células selecionadas.
- **Mostrar ou Ocultar Nota** mantém a nota das células selecionadas sempre visível na planilha, em uma caixa ao lado de cada célula, ou volta a escondê-la.
- **Mostrar Todas as Notas** faz o mesmo para todas as notas da planilha; o mesmo botão passa a se chamar **Ocultar Todas as Notas**.
- **Nota Anterior** e **Próxima Nota** levam de uma célula com nota para a seguinte.
- **Mover ou Redimensionar Nota** move ou altera o tamanho da caixa de uma nota visível usando o teclado. Selecione a célula e use o comando (**Revisão > Notas**, ou clique com o botão direito na célula). Depois, as **teclas de seta** movem a caixa em 10 pixels; **Shift com uma seta** a torna mais larga, mais estreita, mais alta ou mais baixa em 10 pixels; e **Ctrl** (**Option** em um Mac) faz os passos de 1 pixel. Combinações com Alt ou Meta (ou Command, em um Mac) não fazem nada nesse meio-tempo, e qualquer outro comando, a troca de planilha, um Desfazer ou um Refazer cancela a operação. **Enter** mantém a caixa onde está, como um único passo que pode ser desfeito, e **Escape** cancela. Um leitor de tela informa a posição da caixa após cada tecla, em pixels com zoom de 100% (quantos pixels à direita e para baixo do canto superior esquerdo da célula, e a largura e a altura da caixa), e informa quando você solta uma tecla mantida pressionada, não a cada repetição; também informa quando a caixa é mantida ou cancelada. A caixa tem no máximo 640 pixels de largura e 480 de altura, e começa a até 1.000 pixels da sua célula, como ao arrastá-la. Arrastar a caixa e suas oito alças com o ponteiro produz o mesmo resultado.
- **Redefinir Tamanho da Nota** devolve a caixa da nota ao padrão: ao lado da célula, com o tamanho necessário para o texto.

Cada um desses itens é um passo que pode ser desfeito. Uma nota acompanha a sua célula quando linhas ou colunas são inseridas ou excluídas e quando as células são classificadas. **Limpar Tudo** remove as notas e os comentários das células que limpa.

Uma célula também pode ter uma conversa de **comentários**, com respostas. Veja a próxima parte.

As notas podem ser salvas em uma pasta de trabalho .xlsx, .ods ou .xls. Uma pasta de trabalho aberta de um arquivo .ods ou .xls tem suas notas exibidas, e você pode adicionar, alterar, mover e excluir essas notas como em uma pasta .xlsx. A data de uma nota .ods não é mantida quando o JeffCrunch salva. Uma conversa de comentários salva como arquivo .ods ou .xls torna-se uma nota simples com o seu texto, e continua sendo uma nota simples ao abrir esse arquivo. Um arquivo .xls guarda a distância de uma caixa até a célula em um tamanho fixo, por isso uma caixa posicionada longe dentro de uma célula larga pode se deslocar um pouco. Uma pasta de trabalho aberta de um arquivo de texto não recebe notas.

## Comentários

Um **comentário** é parecido com uma nota, mas inicia uma **discussão**: outras pessoas podem responder a ele, e uma delas pode marcar a discussão como **resolvida** quando a dúvida for respondida. Cada comentário mostra quem o escreveu e quando. Uma célula com discussão tem a mesma marca vermelha de uma célula com nota. Aponte para a célula para ler a discussão inteira, com cada resposta recuada abaixo do comentário que ela responde. Uma discussão resolvida começa com a palavra **Resolvida**.

Na guia **Revisão**, no grupo **Comentários**:

- **Novo Comentário** (**Ctrl+Shift+F2**) inicia uma discussão na célula selecionada. Digite o texto e clique em **OK**. Funciona em uma célula que não tem nota nem comentários.
- **Responder** adiciona o seu texto à discussão da célula selecionada.
- **Resolver Discussão** marca a discussão como concluída. O mesmo botão passa a se chamar **Reabrir Discussão**, que a abre novamente.
- **Excluir Comentário** remove as discussões das células selecionadas.
- **Comentário Anterior** e **Próximo Comentário** levam de uma célula com discussão para a seguinte.
- **Mostrar Comentários** abre o painel **Comentários** ao lado da planilha. Ele lista cada discussão da planilha como um cartão: a célula, cada comentário com o autor e a hora, e os botões **Responder**, **Resolver** ou **Reabrir**, e **Excluir**. **Ir Para** vai até a célula. O painel acompanha todas as alterações, inclusive o Desfazer.
- **Converter Notas em Comentários** transforma cada nota simples da planilha em uma discussão de um comentário. **Converter Comentários em Notas** transforma cada discussão de volta em uma nota. Uma nota não tem respostas; por isso, os comentários de uma discussão viram uma única nota com todo o seu texto, e o JeffCrunch pede confirmação antes de fazer isso com uma discussão que tenha respostas.

Cada um desses itens é um passo que pode ser desfeito.

Programas que não conseguem exibir discussões mostram no lugar uma nota simples. O JeffCrunch mantém essa nota, com o texto da discussão, a cada vez que salva. Discussões, respostas e as pessoas que as escreveram permanecem como estavam quando você salva e abre a pasta de trabalho novamente. Uma discussão com mais de 500 comentários é exibida e mantida, mas o JeffCrunch não acrescenta nada a ela nem a altera.

Os comentários são salvos em uma pasta de trabalho .xlsx. Uma pasta de trabalho aberta de um tipo de arquivo mais antigo, ou de um arquivo de texto, mantém os seus comentários somente depois que você a salvar como pasta de trabalho .xlsx.

## Links

Um **link** leva você a outro lugar quando é acionado: a uma página da web, a um arquivo, a um endereço de email ou a um local nesta pasta de trabalho (uma célula, um intervalo de células ou um nome).

Para colocar um link em uma célula, selecione-a e use **Inserir > Link** (ou pressione **Ctrl+K**). Escolha para onde ele leva, digite o endereço ou o local (ou escolha um dos arquivos abertos recentemente em **Ou um arquivo recente**) e, se quiser, o texto que a célula deve mostrar e uma **dica de tela**. Em uma célula que já tem link, o mesmo comando aparece como **Editar Link**. **Remover Link** tira os links das células selecionadas.

Aponte para uma célula com link para ver para onde ele leva. Para segui-lo, mantenha **Ctrl** pressionada e clique na célula, ou selecione a célula e use **Abrir Link** no menu do botão direito. Um clique simples apenas seleciona a célula.

O JeffCrunch nunca abre nada por conta própria. Uma página da web ou um endereço de email abre no seu programa habitual quando você segue o link. Um link para um arquivo, ou para outro tipo de endereço, pede a sua confirmação primeiro. Um link que executaria um script nunca é aberto.

Um link acompanha a sua célula quando linhas ou colunas são inseridas ou excluídas e quando as células são classificadas, e é salvo junto com a pasta de trabalho.

## Estatísticas da Pasta de Trabalho

**Estatísticas da Pasta de Trabalho** (guia **Revisão**) conta o que a pasta de trabalho contém: quantas **Planilhas** ela tem, quantas **Células com dados** (células que contêm um número, um texto ou uma fórmula), quantas **Fórmulas**, quantos **Gráficos** e quantas **Tabelas dinâmicas**. As planilhas que você excluiu não são contadas. Os mesmos números aparecem na página **Informações** da guia Arquivo.

## Ortografia

**Ortografia** (guia **Revisão**, ou **F7**) verifica as palavras das suas células de texto, uma palavra incorreta de cada vez. Se você tiver selecionado mais de uma célula, verifica apenas essas células; caso contrário, verifica a planilha inteira. Não examina fórmulas, números nem datas.

A janela mostra a palavra que não está no dicionário, uma caixa chamada **Alterar para** com a melhor sugestão e uma lista de outras sugestões. Escolha uma sugestão ou digite a sua própria palavra e depois escolha:

- **Alterar** coloca a nova palavra no lugar da errada naquela célula. Cada alteração é um passo que pode ser desfeito.
- **Alterar Tudo** coloca a nova palavra no lugar da mesma palavra em todas as células verificadas, como um único passo que pode ser desfeito.
- **Ignorar** mantém a palavra como está desta vez. **Ignorar Tudo** mantém a palavra em todos os lugares até você fechar o JeffCrunch.
- **Adicionar** coloca a palavra no seu dicionário pessoal, para que nunca mais seja marcada. Seu dicionário é compartilhado com os outros aplicativos do JeffOffice.

**Arquivo > Opções > Revisão de Texto** define se palavras em letras maiúsculas e palavras com números são verificadas. O JeffCrunch verifica no idioma em que é exibido, se tiver um dicionário para ele, e em inglês americano, se não tiver.

Enquanto você digita em uma célula, uma palavra que não está no dicionário recebe um traço vermelho ondulado embaixo. Clique com o botão direito na palavra para escolher uma correção, ou para escolher **Ignorar Tudo** ou **Adicionar ao Dicionário**. As células da grade não são sublinhadas, e um conteúdo que começa com sinal de igual nunca é verificado. Para ver, editar, importar ou exportar a sua lista de palavras adicionadas, e para listar palavras que sempre são marcadas como incorretas, clique em **Arquivo > Opções > Revisão de Texto > Dicionário Personalizado**. Suas listas são as mesmas em todos os aplicativos do JeffOffice.

## Definir o idioma de algumas células

A verificação ortográfica verifica cada célula em seu próprio idioma. Para alterá-lo, selecione as células e clique em **Definir Idioma de Revisão de Texto** no grupo **Idioma** da guia **Revisão**. Escolha o idioma na lista e clique em **OK**. Uma célula que você não marcou é verificada no idioma definido em **Arquivo > Opções**. Uma planilha OpenDocument (.ods) mantém o idioma de cada célula. Uma pasta de trabalho .xlsx não tem onde guardá-lo, por isso o idioma é mantido somente enquanto a pasta de trabalho estiver aberta.

## Não verificar algumas células

Alguns textos não devem ser verificados: um código, um nome ou uma linha em um idioma inventado. Selecione as células e clique em **Não Verificar Ortografia** no grupo **Idioma**. **Ortografia** e os traços vermelhos ondulados na célula passam a ignorá-las. Clique novamente para verificar as células de novo. Um arquivo .ods mantém essa marca, e uma pasta de trabalho .xlsx a mantém somente enquanto a pasta de trabalho estiver aberta.

## Traduzir texto

1. Selecione as células, ou selecione algumas das palavras que você está digitando em uma célula. O texto das células selecionadas é enviado, uma linha por vez.
2. Clique em **Traduzir** no grupo **Idioma**.
3. Escolha o idioma para o qual traduzir.
4. Clique em **OK**.

Seu navegador da web abre então um serviço de tradução chamado LibreTranslate, com o seu texto nele. Nada é enviado até você clicar em **OK**. Você pode alterar o endereço do serviço na caixa, por exemplo para um que você mesmo mantenha. Somente endereços seguros (https) são usados, e endereços http para este computador. São enviadas no máximo 5.000 letras.

## Dicionário de sinônimos

**Dicionário de Sinônimos** (guia **Revisão**, ou **Shift+F7**) encontra palavras de significado quase igual ao de uma palavra da célula. Ele consulta a primeira palavra do texto da célula em que você está. Digite outra palavra em **Pesquisar** e pressione **Enter** para consultar essa outra. Escolha uma palavra na lista e clique em **Substituir**, ou clique duas vezes nela. A palavra na célula é substituída, e uma primeira letra maiúscula continua maiúscula. **Pesquisar** consulta a palavra que você escolheu, para seguir uma cadeia de significados. A lista de palavras funciona em inglês.

## Verificar Acessibilidade

**Verificar Acessibilidade** (guia **Revisão**) abre uma pequena janela que lista o que há na pasta de trabalho e que é difícil de usar para alguns leitores, ou para um leitor de tela (um programa que lê a tela em voz alta). Cada linha começa com **Erro**, **Aviso** ou **Dica**. Clique em uma linha para ir até a célula.

Ele procura:

- Um gráfico ou uma imagem sem **texto alternativo** (as palavras que descrevem o que é mostrado). Isso é um erro.
- Uma tabela sem linha de cabeçalho, de modo que suas colunas não têm nomes. Isso é um erro.
- **Células mescladas**, que são difíceis de percorrer com o teclado. Isso é um aviso.
- Texto difícil de ler contra a cor do fundo, como texto cinza-claro sobre branco. O JeffCrunch mede o quanto o texto se destaca do preenchimento. Textos pequenos precisam de uma proporção de pelo menos 4,5 para 1, e textos grandes (18 pontos, ou negrito de 14 pontos) precisam de 3 para 1. Ele indica a primeira célula de cada combinação de fonte e preenchimento. Isso é um aviso.
- Uma planilha que ainda tem o nome que recebeu, como Planilha1. Isso é uma dica.

Pressione **Verificar Novamente** depois de fazer alterações.
